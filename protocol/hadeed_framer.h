#ifndef HADEED_FRAMER_H
#define HADEED_FRAMER_H

#include <stddef.h>
#include <stdint.h>
#include "hadeed_protocol.h"

#define HADEED_FRAMER_BUFFER_SIZE HADEED_PROTOCOL_MAX_FRAME_SIZE

typedef struct {
    uint8_t buffer[HADEED_FRAMER_BUFFER_SIZE];
    size_t length;
} Hadeed_Framer_t;

typedef void (*Hadeed_Framer_Callback_t)(const Hadeed_Protocol_Frame_t *frame, void *context);

void Hadeed_Framer_Init(Hadeed_Framer_t *framer);
size_t Hadeed_Framer_Feed(Hadeed_Framer_t *framer, const uint8_t *data, size_t length,
                          Hadeed_Framer_Callback_t callback, void *context);

#endif
