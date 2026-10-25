#ifndef HADEED_PROTOCOL_H
#define HADEED_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#define HADEED_PROTOCOL_SOF 0x5Au
#define HADEED_PROTOCOL_VERSION 1u
#define HADEED_PROTOCOL_MSG_SYNC 0x01u
#define HADEED_PROTOCOL_MSG_VISION_RESULT 0x02u
#define HADEED_PROTOCOL_MAX_PAYLOAD 1024u
#define HADEED_PROTOCOL_HEADER_SIZE 13u
#define HADEED_PROTOCOL_CRC_SIZE 2u
#define HADEED_PROTOCOL_MAX_FRAME_SIZE \
    (HADEED_PROTOCOL_HEADER_SIZE + HADEED_PROTOCOL_MAX_PAYLOAD + HADEED_PROTOCOL_CRC_SIZE)

typedef enum {
    HADEED_PROTOCOL_OK = 0,
    HADEED_PROTOCOL_ERR_ARGUMENT = -1,
    HADEED_PROTOCOL_ERR_CAPACITY = -2,
    HADEED_PROTOCOL_ERR_FORMAT = -3,
    HADEED_PROTOCOL_ERR_VERSION = -4,
    HADEED_PROTOCOL_ERR_CRC = -5
} Hadeed_Protocol_Status_t;

typedef struct {
    uint8_t message_type;
    uint32_t sequence;
    uint32_t timestamp_ms;
    const uint8_t *payload;
    uint16_t payload_length;
} Hadeed_Protocol_Frame_t;

uint16_t Hadeed_Protocol_Crc16(const uint8_t *data, size_t length);
Hadeed_Protocol_Status_t Hadeed_Protocol_Encode(
    const Hadeed_Protocol_Frame_t *frame,
    uint8_t *output,
    size_t output_capacity,
    size_t *output_length);
Hadeed_Protocol_Status_t Hadeed_Protocol_Decode(
    const uint8_t *input,
    size_t input_length,
    Hadeed_Protocol_Frame_t *frame,
    const uint8_t **payload);

#endif
