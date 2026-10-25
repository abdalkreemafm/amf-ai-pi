#include <assert.h>
#include <string.h>
#include "hadeed_protocol.h"

int main(void) {
    const uint8_t payload[] = {0x10, 0x20, 0x30};
    uint8_t encoded[HADEED_PROTOCOL_MAX_FRAME_SIZE];
    size_t encoded_length = 0;
    Hadeed_Protocol_Frame_t input = {
        .message_type = HADEED_PROTOCOL_MSG_VISION_RESULT,
        .sequence = 7u,
        .timestamp_ms = 1000u,
        .payload = payload,
        .payload_length = (uint16_t)sizeof(payload),
    };
    assert(Hadeed_Protocol_Encode(&input, encoded, sizeof(encoded), &encoded_length) == HADEED_PROTOCOL_OK);

    Hadeed_Protocol_Frame_t output;
    const uint8_t *decoded_payload = NULL;
    assert(Hadeed_Protocol_Decode(encoded, encoded_length, &output, &decoded_payload) == HADEED_PROTOCOL_OK);
    assert(output.message_type == input.message_type);
    assert(output.sequence == input.sequence);
    assert(output.timestamp_ms == input.timestamp_ms);
    assert(output.payload_length == input.payload_length);
    assert(decoded_payload != NULL && memcmp(decoded_payload, payload, sizeof(payload)) == 0);

    encoded[encoded_length - 1u] ^= 1u;
    assert(Hadeed_Protocol_Decode(encoded, encoded_length, &output, &decoded_payload) == HADEED_PROTOCOL_ERR_CRC);
    return 0;
}
