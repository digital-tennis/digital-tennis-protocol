namespace DigitalTennis.Protocol
{
    // GATT 6d7c1003, read only after encryption. No keys, addresses or PINs on this wire value.
    public readonly struct LinkSecurity
    {
        public readonly byte Level;
        public readonly bool Bonded,SecureConnections;
        public bool Ready=>Level>=2&&Bonded;
        public LinkSecurity(byte level,bool bonded,bool secureConnections){Level=level;Bonded=bonded;SecureConnections=secureConnections;}
        public static DecodeError Decode(byte[] b,out LinkSecurity value)
        {
            value=default;
            if(b==null||b.Length!=4)return DecodeError.InvalidLength;
            if(b[0]!=0x44)return DecodeError.InvalidMagic;
            if(b[1]!=1)return DecodeError.UnsupportedVersion;
            if(b[2]<1||b[2]>4)return DecodeError.OutOfRange;
            if((b[3]&~3)!=0)return DecodeError.InvalidFlags;
            if(b[2]<2&&b[3]!=0)return DecodeError.InvalidFields;
            value=new LinkSecurity(b[2],(b[3]&1)!=0,(b[3]&2)!=0);return DecodeError.None;
        }
    }
}
