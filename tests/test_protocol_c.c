#include <assert.h>
#include <string.h>
#include "hadeed_framer.h"

struct callback_state {
    unsigned count;
    uint32_t last_sequence;
};

static void on_frame(const Hadeed_Protocol_Frame_t *frame, void *context) {
    struct callback_state *state = (struct callback_state *)context;
    assert(frame != NULL && state != NULL);
    state->count++;
    state->last_sequence = frame->sequence;
}

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
    encoded[encoded_length - 1u] ^= 1u;

    Hadeed_Framer_t framer;
    struct callback_state state = {0u, 0u};
    Hadeed_Framer_Init(&framer);
    uint8_t noise_and_partial[] = {0x00u, 0xFFu, encoded[0], encoded[1], encoded[2], encoded[3]};
    assert(Hadeed_Framer_Feed(&framer, noise_and_partial, sizeof(noise_and_partial), on_frame, &state) == 0u);
    assert(Hadeed_Framer_Feed(&framer, &encoded[4], encoded_length - 4u, on_frame, &state) == 1u);
    assert(state.count == 1u && state.last_sequence == 7u);

    uint8_t two_frames[HADEED_PROTOCOL_MAX_FRAME_SIZE * 2u];
    memcpy(two_frames, encoded, encoded_length);
    Hadeed_Protocol_Frame_t second = input;
    second.sequence = 8u;
    size_t second_length = 0u;
    assert(Hadeed_Protocol_Encode(&second, &two_frames[encoded_length],
                                  sizeof(two_frames) - encoded_length, &second_length) == HADEED_PROTOCOL_OK);
    assert(Hadeed_Framer_Feed(&framer, two_frames, encoded_length + second_length, on_frame, &state) == 2u);
    assert(state.count == 3u && state.last_sequence == 8u);
    return 0;
}
