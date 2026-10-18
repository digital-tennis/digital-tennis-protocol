#!/usr/bin/env python3
"""Run the same shared and generated wire corpus through independent C and C# codecs."""
import json, pathlib, random, subprocess
root=pathlib.Path(__file__).resolve().parents[1]
out=root/'.work';out.mkdir(exist_ok=True)
subprocess.run(['cc','-std=c99','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I',str(root/'c/include'),str(root/'c/src/dt_protocol.c'),str(root/'tests/codec_cli.c'),'-o',str(out/'codec')],check=True)
subprocess.run(['dotnet','build',str(root/'tests/Checks.csproj'),'-c','Release','--nologo','-v','quiet'],check=True)
spec=json.loads((root/'test-vectors.json').read_text())
names={'INVALID_LENGTH':1,'INVALID_MAGIC':2,'UNSUPPORTED_VERSION':3,'BAD_CRC':4,'INVALID_TYPE':5,'INVALID_FLAGS':6,'INVALID_KIND':7,'OUT_OF_RANGE':8,'INVALID_FIELDS':9,None:0}
cases=[(x['name'],bytes.fromhex(x['hex']),names[x['expected_error']],x.get('expected') or {}) for x in spec['cases']]
def crc(b):
    c=0
    for value in b:
        c^=value
        for _ in range(8):c=((c<<1)^7 if c&128 else c<<1)&255
    return c
rng=random.Random(107)
for version in (1,2):
    for kind in range(1,9):
        b=bytearray.fromhex(spec['cases'][0]['hex']);b[1]=version;b[12]=kind;b[19]=crc(b[:19]);cases.append((f'v{version}-kind{kind}',bytes(b),0 if version==2 or kind<=3 else 7,{}))
for i in range(1000):
    b=bytearray.fromhex(spec['cases'][0]['hex']);b[1]=2;b[4:12]=rng.randbytes(8);b[12]=rng.randint(1,8);b[13]=rng.randint(0,100);b[14:16]=rng.randint(-18000,18000).to_bytes(2,'little',signed=True);b[16:18]=rng.randint(-9000,9000).to_bytes(2,'little',signed=True);b[19]=crc(b[:19]);cases.append((f'generated-{i}',bytes(b),0,{}))
    damaged=bytearray(b);damaged[rng.randint(2,18)]^=1;cases.append((f'corrupt-{i}',bytes(damaged),4,{}))
wire=''.join(b.hex(' ')+'\n' for _,b,_,_ in cases)
c=subprocess.run([str(out/'codec')],input=wire,text=True,capture_output=True,check=True).stdout.splitlines()
cs=subprocess.run(['dotnet',str(root/'tests/bin/Release/net10.0/Checks.dll')],input=wire,text=True,capture_output=True,check=True).stdout.splitlines()
assert len(c)==len(cs)==len(cases)
fields=['version','type','flags','sequence','deviceTimeMs','eventId','kind','power','yawCdeg','pitchCdeg','battery']
for case,left,right in zip(cases,c,cs):
    name,_,expected,values=case
    assert left==right,(name,'C/C# mismatch',left,right)
    assert int(left.split('|')[0])==expected,(name,expected,left)
    if not expected:
        decoded=dict(zip(fields,map(int,left.split('|')[1].split(','))))
        for key,value in values.items():
            if key in decoded:assert decoded[key]==value,(name,key,decoded[key],value)
report={'passed':True,'cases':len(cases),'sharedCases':len(spec['cases']),'generatedSeed':107,'sanitizers':['address','undefined'],'languages':['C99','C# netstandard2.1']}
(out/'validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
