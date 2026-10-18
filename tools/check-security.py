#!/usr/bin/env python3
"""Cross-check every secure-link level/flags pair in C and C#."""
import pathlib,subprocess,json
p=pathlib.Path(__file__).resolve().parents[1];out=p/'.work';out.mkdir(exist_ok=True)
subprocess.run(['cc','-std=c99','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(p/'c/include'),str(p/'c/src/dt_protocol.c'),str(p/'tests/security_cli.c'),'-o',str(out/'security')],check=True)
subprocess.run(['dotnet','build',str(p/'tests/SecurityChecks.csproj'),'-c','Release','--nologo','-v','quiet'],check=True)
cases=[]
for level in range(256):
 for flags in range(256):
  error=8 if level<1 or level>4 else 6 if flags&~3 else 9 if level<2 and flags else 0
  cases.append((bytes([0x44,1,level,flags]),error))
for magic in range(256):
 if magic!=0x44:cases.append((bytes([magic,1,2,3]),2))
for version in range(256):
 if version!=1:cases.append((bytes([0x44,version,2,3]),3))
wire=''.join(b.hex(' ')+'\n' for b,e in cases)
a=subprocess.run([str(out/'security')],input=wire,text=True,capture_output=True,check=True).stdout.splitlines()
b=subprocess.run(['dotnet',str(p/'tests/bin/Release/net10.0/SecurityChecks.dll')],input=wire,text=True,capture_output=True,check=True).stdout.splitlines()
assert len(a)==len(b)==len(cases)
for i,(_,expected) in enumerate(cases):assert a[i]==b[i]==str(expected),(i,a[i],b[i],expected)
r={'passed':True,'cases':len(cases),'scope':'Secure-link GATT C/C# grammar, no physical bonding proof'};(out/'security-validation.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r))
