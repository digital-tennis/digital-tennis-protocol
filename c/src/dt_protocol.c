#include "dt_protocol.h"
#include <string.h>
uint8_t dt_crc8(const uint8_t *b,size_t n){uint8_t c=0;for(size_t i=0;i<n;i++){c^=b[i];for(int j=0;j<8;j++)c=(uint8_t)((c&128)?(c<<1)^7:c<<1);}return c;}
int dt_sequence_newer(uint16_t a,uint16_t b){uint16_t d=(uint16_t)(a-b);return d>0&&d<32768;}
dt_error dt_validate(const dt_packet *p){
 if(!p)return DT_FIELDS;
 if(p->version!=1&&p->version!=2)return DT_VERSION;
 if(p->flags&~7)return DT_FLAGS;
 if((p->battery>100&&p->battery!=255)||p->power>100||p->yaw_cdeg < -18000||p->yaw_cdeg>18000||p->pitch_cdeg < -9000||p->pitch_cdeg>9000)return DT_RANGE;
 switch(p->type){case 1:if(p->kind<1||p->kind>(p->version==1?3:8))return DT_KIND;break;case 2:if(p->kind<1||p->kind>3)return DT_KIND;break;case 3:case 128:if(p->kind)return DT_KIND;break;case 129:if(p->kind>3)return DT_KIND;break;case 130:case 131:if(p->version!=2)return DT_TYPE;if(p->kind)return DT_KIND;if(!p->event_id)return DT_FIELDS;break;default:return DT_TYPE;}
 if(p->type!=1&&(p->power||p->yaw_cdeg||p->pitch_cdeg))return DT_FIELDS;
 if(p->type==3&&p->event_id)return DT_FIELDS;
 if((p->type==128||p->type==130)&&(p->flags||p->device_ms||p->battery!=255))return DT_FIELDS;
 return DT_OK;
}
static uint16_t u16(const uint8_t *b){return (uint16_t)(b[0]|b[1]<<8);}
static uint32_t u32(const uint8_t *b){return (uint32_t)b[0]|(uint32_t)b[1]<<8|(uint32_t)b[2]<<16|(uint32_t)b[3]<<24;}
dt_error dt_decode(const uint8_t *b,size_t n,dt_packet *p){
 if(!p){return DT_FIELDS;}
 memset(p,0,sizeof(*p));
 if(!b||n!=20){return DT_LENGTH;}
 if(b[0]!=0x44){return DT_MAGIC;}
 if(b[1]!=1&&b[1]!=2){return DT_VERSION;}
 if(dt_crc8(b,19)!=b[19]){return DT_CRC;}
 dt_packet v={b[1],b[2],b[3],u16(b+4),u32(b+6),u16(b+10),b[12],b[13],(int16_t)u16(b+14),(int16_t)u16(b+16),b[18]};
 dt_error e=dt_validate(&v);if(e==DT_OK)*p=v;return e;
}
dt_error dt_encode(const dt_packet *p,uint8_t *b,size_t n){
 if(!b||n!=20){return DT_LENGTH;}
 dt_error e=dt_validate(p);if(e!=DT_OK){return e;}
 b[0]=0x44;b[1]=p->version;b[2]=p->type;b[3]=p->flags;b[4]=(uint8_t)p->sequence;b[5]=(uint8_t)(p->sequence>>8);
 for(int j=0;j<4;j++){b[6+j]=(uint8_t)(p->device_ms>>(8*j));}
 b[10]=(uint8_t)p->event_id;b[11]=(uint8_t)(p->event_id>>8);b[12]=p->kind;b[13]=p->power;
 uint16_t yaw=(uint16_t)p->yaw_cdeg,pitch=(uint16_t)p->pitch_cdeg;b[14]=(uint8_t)yaw;b[15]=(uint8_t)(yaw>>8);b[16]=(uint8_t)pitch;b[17]=(uint8_t)(pitch>>8);b[18]=p->battery;b[19]=dt_crc8(b,19);return DT_OK;
}
