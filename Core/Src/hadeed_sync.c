#include "hadeed_sync.h"
#include "main.h"
#include <string.h>

void Hadeed_Sync_Init(Hadeed_Sync_Manager_t *sm) {
    if (sm == NULL) return;
    memset(sm, 0, sizeof(*sm));
    sm->state = SYNC_STATE_DISCONNECTED;
}

void Hadeed_Sync_PreparePacket(Hadeed_Sync_Packet_t *packet) {
    static uint32_t sequence;
    if (packet == NULL) return;
    packet->master_timestamp = HAL_GetTick();
    packet->sequence_id = ++sequence;
    packet->system_state = 0u;
}

size_t Hadeed_Sync_EncodeFrame(const Hadeed_Sync_Packet_t *packet, uint8_t *output, size_t capacity) {
    if (packet == NULL || output == NULL) return 0u;
    const uint8_t payload[] = {packet->system_state};
    Hadeed_Protocol_Frame_t frame = {
        .message_type = HADEED_PROTOCOL_MSG_SYNC,
        .sequence = packet->sequence_id,
        .timestamp_ms = packet->master_timestamp,
        .payload = payload,
        .payload_length = sizeof(payload),
    };
    size_t length = 0u;
    if (Hadeed_Protocol_Encode(&frame, output, capacity, &length) != HADEED_PROTOCOL_OK) return 0u;
    return length;
}

bool Hadeed_Sync_DecodeVisionFrame(const uint8_t *input, size_t input_length, Hadeed_K210_Response_t *response) {
    if (response == NULL) return false;
    Hadeed_Protocol_Frame_t frame;
    const uint8_t *payload = NULL;
    if (Hadeed_Protocol_Decode(input, input_length, &frame, &payload) != HADEED_PROTOCOL_OK ||
        frame.message_type != HADEED_PROTOCOL_MSG_VISION_RESULT || frame.payload_length != 4u) return false;
    response->slave_timestamp = frame.timestamp_ms;
    response->detected_class = payload[0];
    response->confidence = payload[1];
    response->inference_time_ms = (uint16_t)(((uint16_t)payload[2] << 8) | payload[3]);
    return true;
}

void Hadeed_Sync_ProcessResponse(Hadeed_Sync_Manager_t *sm, const Hadeed_K210_Response_t *response) {
    if (sm == NULL || response == NULL) return;
    sm->last_sync_time = HAL_GetTick();
    sm->state = SYNC_STATE_LOCKED;
    sm->missed_heartbeats = 0u;
}

bool Hadeed_Sync_IsLocked(const Hadeed_Sync_Manager_t *sm) {
    return sm != NULL && sm->state == SYNC_STATE_LOCKED;
}
