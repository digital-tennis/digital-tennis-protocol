#include "dt_protocol.h"
#include <stdio.h>
#include <string.h>
int main(void){
 if(dt_crc8((const uint8_t*)"123456789",9)!=0xf4||!dt_sequence_newer(0,65535)||dt_sequence_newer(42,42)||dt_sequence_newer(32778,10))return 2;
 char line[8192];while(fgets(line,sizeof(line),stdin)){
  uint8_t b[2048];size_t n=0;unsigned v;char *s=line;int consumed=0;
  while(n<sizeof(b)&&sscanf(s,"%2x%n",&v,&consumed)==1){b[n++]=(uint8_t)v;s+=consumed;}
  dt_packet p;dt_error e=dt_decode(b,n,&p);printf("%d",e);
  if(e==DT_OK){uint8_t encoded[20];if(dt_encode(&p,encoded,20)!=DT_OK||memcmp(b,encoded,20))return 3;printf("|%u,%u,%u,%u,%u,%u,%u,%u,%d,%d,%u",p.version,p.type,p.flags,p.sequence,p.device_ms,p.event_id,p.kind,p.power,p.yaw_cdeg,p.pitch_cdeg,p.battery);}
  putchar('\n');
 }return 0;
}
