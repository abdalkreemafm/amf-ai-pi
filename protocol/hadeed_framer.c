#include "hadeed_framer.h"
#include <string.h>

static void drop_prefix(Hadeed_Framer_t *framer, size_t count) {
    if (count >= framer->length) {
        framer->length = 0u;
        return;
    }
    memmove(framer->buffer, &framer->buffer[count], framer->length - count);
    framer->length -= count;
}

void Hadeed_Framer_Init(Hadeed_Framer_t *framer) {
    if (framer != NULL) framer->length = 0u;
}

size_t Hadeed_Framer_Feed(Hadeed_Framer_t *framer, const uint8_t *data, size_t length,
                          Hadeed_Framer_Callback_t callback, void *context) {
    if (framer == NULL || (data == NULL && length != 0u)) return 0u;
    size_t emitted = 0u;
    for (size_t input_index = 0; input_index < length; ++input_index) {
        uint8_t byte = data[input_index];
        if (framer->length == 0u && byte != HADEED_PROTOCOL_SOF) continue;
        if (framer->length >= sizeof(framer->buffer)) {
            drop_prefix(framer, 1u);
        }
        framer->buffer[framer->length++] = byte;

        for (;;) {
            if (framer->length == 0u || framer->buffer[0] != HADEED_PROTOCOL_SOF) {
                if (framer->length != 0u) drop_prefix(framer, 1u);
                break;
            }
            if (framer->length < HADEED_PROTOCOL_HEADER_SIZE) break;
            uint16_t payload_length = (uint16_t)(((uint16_t)framer->buffer[3] << 8) | framer->buffer[4]);
            if (payload_length > HADEED_PROTOCOL_MAX_PAYLOAD) {
                drop_prefix(framer, 1u);
                continue;
            }
            size_t frame_length = HADEED_PROTOCOL_HEADER_SIZE + payload_length + HADEED_PROTOCOL_CRC_SIZE;
            if (framer->length < frame_length) break;

            Hadeed_Protocol_Frame_t frame;
            const uint8_t *payload = NULL;
            if (Hadeed_Protocol_Decode(framer->buffer, frame_length, &frame, &payload) == HADEED_PROTOCOL_OK) {
                if (callback != NULL) callback(&frame, context);
                ++emitted;
                drop_prefix(framer, frame_length);
            } else {
                drop_prefix(framer, 1u);
            }
        }
    }
    return emitted;
}
