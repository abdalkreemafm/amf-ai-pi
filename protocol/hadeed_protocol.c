#include "hadeed_protocol.h"

static void put_u16_be(uint8_t *p, uint16_t value) {
    p[0] = (uint8_t)(value >> 8);
    p[1] = (uint8_t)value;
}

static void put_u32_be(uint8_t *p, uint32_t value) {
    p[0] = (uint8_t)(value >> 24);
    p[1] = (uint8_t)(value >> 16);
    p[2] = (uint8_t)(value >> 8);
    p[3] = (uint8_t)value;
}

static uint16_t get_u16_be(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t get_u32_be(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

uint16_t Hadeed_Protocol_Crc16(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFFu;
    if (data == NULL) return 0u;
    for (size_t i = 0; i < length; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x8000u) ? (uint16_t)((crc << 1) ^ 0x1021u)
                                  : (uint16_t)(crc << 1);
        }
    }
    return crc;
}

Hadeed_Protocol_Status_t Hadeed_Protocol_Encode(
    const Hadeed_Protocol_Frame_t *frame,
    uint8_t *output,
    size_t output_capacity,
    size_t *output_length) {
    if (frame == NULL || output == NULL || output_length == NULL) return HADEED_PROTOCOL_ERR_ARGUMENT;
    if (frame->payload_length > HADEED_PROTOCOL_MAX_PAYLOAD ||
        (frame->payload_length > 0u && frame->payload == NULL)) return HADEED_PROTOCOL_ERR_ARGUMENT;
    size_t total = HADEED_PROTOCOL_HEADER_SIZE + frame->payload_length + HADEED_PROTOCOL_CRC_SIZE;
    if (output_capacity < total) return HADEED_PROTOCOL_ERR_CAPACITY;

    output[0] = HADEED_PROTOCOL_SOF;
    output[1] = HADEED_PROTOCOL_VERSION;
    output[2] = frame->message_type;
    put_u16_be(&output[3], frame->payload_length);
    put_u32_be(&output[5], frame->sequence);
    put_u32_be(&output[9], frame->timestamp_ms);
    for (uint16_t i = 0; i < frame->payload_length; ++i) output[13u + i] = frame->payload[i];
    put_u16_be(&output[13u + frame->payload_length], Hadeed_Protocol_Crc16(&output[1], 12u + frame->payload_length));
    *output_length = total;
    return HADEED_PROTOCOL_OK;
}

Hadeed_Protocol_Status_t Hadeed_Protocol_Decode(
    const uint8_t *input,
    size_t input_length,
    Hadeed_Protocol_Frame_t *frame,
    const uint8_t **payload) {
    if (input == NULL || frame == NULL || payload == NULL) return HADEED_PROTOCOL_ERR_ARGUMENT;
    if (input_length < HADEED_PROTOCOL_HEADER_SIZE + HADEED_PROTOCOL_CRC_SIZE) return HADEED_PROTOCOL_ERR_FORMAT;
    if (input[0] != HADEED_PROTOCOL_SOF) return HADEED_PROTOCOL_ERR_FORMAT;
    if (input[1] != HADEED_PROTOCOL_VERSION) return HADEED_PROTOCOL_ERR_VERSION;
    uint16_t payload_length = get_u16_be(&input[3]);
    if (payload_length > HADEED_PROTOCOL_MAX_PAYLOAD ||
        input_length != HADEED_PROTOCOL_HEADER_SIZE + payload_length + HADEED_PROTOCOL_CRC_SIZE) return HADEED_PROTOCOL_ERR_FORMAT;
    uint16_t expected = get_u16_be(&input[13u + payload_length]);
    if (expected != Hadeed_Protocol_Crc16(&input[1], 12u + payload_length)) return HADEED_PROTOCOL_ERR_CRC;

    frame->message_type = input[2];
    frame->payload_length = payload_length;
    frame->sequence = get_u32_be(&input[5]);
    frame->timestamp_ms = get_u32_be(&input[9]);
    *payload = &input[13];
    frame->payload = *payload;
    return HADEED_PROTOCOL_OK;
}
