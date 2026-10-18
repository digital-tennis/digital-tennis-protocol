using System;
using System.Linq;
using System.Globalization;
using DigitalTennis.Protocol;
CultureInfo.CurrentCulture=CultureInfo.InvariantCulture;
if(PacketCodec.Crc(System.Text.Encoding.ASCII.GetBytes("123456789"),9)!=0xf4||!PacketCodec.Newer(0,65535)||PacketCodec.Newer(42,42)||PacketCodec.Newer(32778,10))return 2;
string line;while((line=Console.ReadLine())!=null){var bytes=line.Split(' ',StringSplitOptions.RemoveEmptyEntries).Select(x=>Convert.ToByte(x,16)).ToArray();var error=PacketCodec.Decode(bytes,out var p);Console.Write((int)error);if(error==DecodeError.None){if(!PacketCodec.Encode(p).SequenceEqual(bytes))return 3;Console.Write($"|{p.Version},{(byte)p.Type},{p.Flags},{p.Sequence},{p.DeviceTimeMs},{p.EventId},{p.Kind},{p.Power},{p.YawCdeg},{p.PitchCdeg},{p.Battery}");}Console.WriteLine();}return 0;
