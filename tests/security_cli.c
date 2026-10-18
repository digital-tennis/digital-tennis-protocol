#include "dt_protocol.h"
#include <stdio.h>
int main(void){dt_link_security v={2,1,1};uint8_t z[4]={0x44,1,2,3};if(dt_security_decode(NULL,0,&v)!=DT_LENGTH||dt_security_decode(z,3,&v)!=DT_LENGTH||dt_security_decode(z,4,NULL)!=DT_FIELDS||dt_security_encode(&v,z,3)!=DT_LENGTH)return 4;unsigned a,b,c,d;while(scanf("%x %x %x %x",&a,&b,&c,&d)==4){uint8_t bytes[]={a,b,c,d};dt_link_security s;dt_error e=dt_security_decode(bytes,4,&s);if(e==DT_OK){uint8_t out[4];if(dt_security_encode(&s,out,4)!=DT_OK)return 2;for(int i=0;i<4;i++)if(out[i]!=bytes[i])return 3;}printf("%d\n",e);}return 0;}
