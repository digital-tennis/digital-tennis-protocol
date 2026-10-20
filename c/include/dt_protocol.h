#ifndef DT_PROTOCOL_H
#define DT_PROTOCOL_H
#include <stddef.h>
#include <stdint.h>
typedef enum { DT_OK,DT_LENGTH,DT_MAGIC,DT_VERSION,DT_CRC,DT_TYPE,DT_FLAGS,DT_KIND,DT_RANGE,DT_FIELDS } dt_error;
typedef enum { DT_INPUT_MOTION=0, DT_INPUT_BUTTONS=1, DT_INPUT_PHYSICAL_MOTION=2, DT_INPUT_PHYSICAL_BUTTONS=3 } dt_input_profile;
typedef enum { DT_MOTION_HORIZONTAL_POSITIVE=1, DT_MOTION_HORIZONTAL_NEGATIVE=2, DT_MOTION_OVERHEAD_DOWN=3, DT_MOTION_SHORT_ROTATION=4, DT_MOTION_OBLIQUE_DOWN=5, DT_MOTION_OBLIQUE_UP=6 } dt_motion_pattern;
enum { DT_PACKET_MOTION=4, DT_PACKET_BUTTON_DOWN=5, DT_PACKET_BUTTON_UP=6 };
typedef enum { DT_BUTTON_CONFIRM=1, DT_BUTTON_START=2, DT_BUTTON_PAUSE=3, DT_BUTTON_SERVE=4, DT_BUTTON_FOREHAND=5, DT_BUTTON_BACKHAND=6, DT_BUTTON_SKILL=7 } dt_button_kind;
typedef struct { uint8_t version,type,flags; uint16_t sequence; uint32_t device_ms; uint16_t event_id; uint8_t kind,power; int16_t yaw_cdeg,pitch_cdeg; uint8_t battery; } dt_packet;
uint8_t dt_crc8(const uint8_t *bytes,size_t length);
int dt_sequence_newer(uint16_t value,uint16_t previous);
dt_error dt_validate(const dt_packet *packet);
dt_error dt_decode(const uint8_t *bytes,size_t length,dt_packet *packet);
dt_error dt_encode(const dt_packet *packet,uint8_t *bytes,size_t length);
/* Read-only secure-link GATT value: magic 0x44, schema 1, level 1..4, flags bonded=1/SC=2. */
typedef struct {uint8_t level;uint8_t bonded;uint8_t secure_connections;} dt_link_security;
dt_error dt_security_decode(const uint8_t *bytes,size_t length,dt_link_security *status);
dt_error dt_security_encode(const dt_link_security *status,uint8_t *bytes,size_t length);
#endif
