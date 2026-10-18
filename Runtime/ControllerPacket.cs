using System;

namespace DigitalTennis.Protocol
{
    public enum PacketType : byte { Swing=1, Button=2, Status=3, Calibrate=128, Ack=129, LinkProbe=130, LinkReply=131 }
    // v1 IDs remain unchanged. Additional explicit intents require v2.
    public enum SwingKind : byte { Forehand=1, Backhand=2, Serve=3, Smash=4, Volley=5, Slice=6, Lob=7, Skill=8 }
    public enum DecodeError { None, InvalidLength, InvalidMagic, UnsupportedVersion, BadCrc, InvalidType, InvalidFlags, InvalidKind, OutOfRange, InvalidFields }
    public readonly struct ControllerPacket
    {
        public readonly byte Version,Flags,Kind,Power,Battery;
        public readonly PacketType Type;
        public readonly ushort Sequence,EventId;
        public readonly uint DeviceTimeMs;
        public readonly short YawCdeg,PitchCdeg;
        public ControllerPacket(byte version,PacketType type,byte flags,ushort sequence,uint deviceTimeMs,ushort eventId,byte kind,byte power,short yawCdeg,short pitchCdeg,byte battery)
        {Version=version;Type=type;Flags=flags;Sequence=sequence;DeviceTimeMs=deviceTimeMs;EventId=eventId;Kind=kind;Power=power;YawCdeg=yawCdeg;PitchCdeg=pitchCdeg;Battery=battery;}
        public bool MotionReady=>Type==PacketType.Swing&&(Flags&3)==3;
    }
    public static class PacketCodec
    {
        public const int Size=20;
        public static byte Crc(byte[] bytes,int count)
        {
            if(bytes==null||count<0||count>bytes.Length)throw new ArgumentOutOfRangeException(nameof(count));
            byte crc=0;for(int i=0;i<count;i++){crc^=bytes[i];for(int bit=0;bit<8;bit++)crc=(byte)((crc&128)!=0?(crc<<1)^7:crc<<1);}return crc;
        }
        public static bool Newer(ushort value,ushort previous){int delta=(value-previous)&65535;return delta>0&&delta<32768;}
        public static DecodeError Validate(ControllerPacket p)
        {
            if(p.Version!=1&&p.Version!=2)return DecodeError.UnsupportedVersion;
            if((p.Flags&~7)!=0)return DecodeError.InvalidFlags;
            if(p.Battery>100&&p.Battery!=255||p.Power>100||p.YawCdeg < -18000||p.YawCdeg>18000||p.PitchCdeg < -9000||p.PitchCdeg>9000)return DecodeError.OutOfRange;
            switch(p.Type)
            {
                case PacketType.Swing:if(p.Kind<1||p.Kind>(p.Version==1?3:8))return DecodeError.InvalidKind;break;
                case PacketType.Button:if(p.Kind<1||p.Kind>3)return DecodeError.InvalidKind;break;
                case PacketType.Status:case PacketType.Calibrate:if(p.Kind!=0)return DecodeError.InvalidKind;break;
                case PacketType.Ack:if(p.Kind>3)return DecodeError.InvalidKind;break;
                case PacketType.LinkProbe:case PacketType.LinkReply:if(p.Version!=2)return DecodeError.InvalidType;if(p.Kind!=0)return DecodeError.InvalidKind;if(p.EventId==0)return DecodeError.InvalidFields;break;
                default:return DecodeError.InvalidType;
            }
            if(p.Type!=PacketType.Swing&&(p.Power!=0||p.YawCdeg!=0||p.PitchCdeg!=0))return DecodeError.InvalidFields;
            if(p.Type==PacketType.Status&&p.EventId!=0)return DecodeError.InvalidFields;
            if((p.Type==PacketType.Calibrate||p.Type==PacketType.LinkProbe)&&(p.Flags!=0||p.DeviceTimeMs!=0||p.Battery!=255))return DecodeError.InvalidFields;
            return DecodeError.None;
        }
        public static DecodeError Decode(byte[] data,out ControllerPacket packet)
        {
            packet=default;if(data==null||data.Length!=Size)return DecodeError.InvalidLength;
            if(data[0]!=0x44)return DecodeError.InvalidMagic;
            if(data[1]!=1&&data[1]!=2)return DecodeError.UnsupportedVersion;
            if(Crc(data,19)!=data[19])return DecodeError.BadCrc;
            var p=new ControllerPacket(data[1],(PacketType)data[2],data[3],U16(data,4),U32(data,6),U16(data,10),data[12],data[13],unchecked((short)U16(data,14)),unchecked((short)U16(data,16)),data[18]);
            var error=Validate(p);if(error==DecodeError.None)packet=p;return error;
        }
        public static byte[] Encode(ControllerPacket p)
        {
            var error=Validate(p);if(error!=DecodeError.None)throw new ArgumentException(error.ToString(),nameof(p));
            var b=new byte[Size];b[0]=0x44;b[1]=p.Version;b[2]=(byte)p.Type;b[3]=p.Flags;W16(b,4,p.Sequence);W32(b,6,p.DeviceTimeMs);W16(b,10,p.EventId);b[12]=p.Kind;b[13]=p.Power;W16(b,14,unchecked((ushort)p.YawCdeg));W16(b,16,unchecked((ushort)p.PitchCdeg));b[18]=p.Battery;b[19]=Crc(b,19);return b;
        }
        private static ushort U16(byte[] b,int i)=>(ushort)(b[i]|b[i+1]<<8);
        private static uint U32(byte[] b,int i)=>(uint)(b[i]|b[i+1]<<8|b[i+2]<<16|b[i+3]<<24);
        private static void W16(byte[] b,int i,ushort v){b[i]=(byte)v;b[i+1]=(byte)(v>>8);}
        private static void W32(byte[] b,int i,uint v){for(int j=0;j<4;j++)b[i+j]=(byte)(v>>(8*j));}
    }
}
