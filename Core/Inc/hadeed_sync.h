#ifndef HADEED_SYNC_H
#define HADEED_SYNC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "hadeed_protocol.h"

#define HADEED_SYNC_SOF HADEED_PROTOCOL_SOF
#define MAX_ALLOWED_DRIFT_MS 5u
#define HEARTBEAT_INTERVAL_MS 100u

typedef enum {
    SYNC_STATE_DISCONNECTED,
    SYNC_STATE_HANDSHAKING,
    SYNC_STATE_LOCKED,
    SYNC_STATE_DRIFT_WARN,
    SYNC_STATE_ERROR
} Hadeed_Sync_State_t;

/* Logical fields only. Wire encoding is performed by hadeed_sync.c. */
typedef struct {
    uint32_t master_timestamp;
    uint32_t sequence_id;
    uint8_t system_state;
} Hadeed_Sync_Packet_t;

typedef struct {
    uint32_t slave_timestamp;
    uint8_t detected_class;
    uint8_t confidence;
    uint16_t inference_time_ms;
} Hadeed_K210_Response_t;

typedef struct {
    Hadeed_Sync_State_t state;
    uint32_t last_sync_time;
    uint32_t round_trip_latency_ms;
    int32_t clock_offset;
    uint32_t missed_heartbeats;
} Hadeed_Sync_Manager_t;

void Hadeed_Sync_Init(Hadeed_Sync_Manager_t *sm);
void Hadeed_Sync_PreparePacket(Hadeed_Sync_Packet_t *packet);
size_t Hadeed_Sync_EncodeFrame(const Hadeed_Sync_Packet_t *packet, uint8_t *output, size_t capacity);
bool Hadeed_Sync_DecodeVisionFrame(const uint8_t *input, size_t input_length, Hadeed_K210_Response_t *response);
void Hadeed_Sync_ProcessResponse(Hadeed_Sync_Manager_t *sm, const Hadeed_K210_Response_t *response);
bool Hadeed_Sync_IsLocked(const Hadeed_Sync_Manager_t *sm);

#endif
