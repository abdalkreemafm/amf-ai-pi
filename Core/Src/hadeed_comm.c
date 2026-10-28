#include "hadeed_math.h"
#include "hadeed_comm.h"
#include "hadeed_logic.h"
#include "hadeed_config.h"
#include "hadeed_protocol.h"
#include "main.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static uint8_t tx_buffer[HADEED_PROTOCOL_MAX_FRAME_SIZE];

static void put_i16_be(uint8_t *out, int16_t value) {
    uint16_t raw = (uint16_t)value;
    out[0] = (uint8_t)(raw >> 8);
    out[1] = (uint8_t)raw;
}

static bool send_frame(uint8_t message_type, const uint8_t *payload, uint16_t payload_length) {
    Hadeed_Protocol_Frame_t frame = {
        .message_type = message_type,
        .sequence = HAL_GetTick(),
        .timestamp_ms = HAL_GetTick(),
        .payload = payload,
        .payload_length = payload_length,
    };
    size_t length = 0u;
    if (Hadeed_Protocol_Encode(&frame, tx_buffer, sizeof(tx_buffer), &length) != HADEED_PROTOCOL_OK) return false;
    return HAL_UART_Transmit_DMA(&huart1, tx_buffer, (uint16_t)length) == HAL_OK;
}

void Hadeed_Comm_Send_Target_Update(const Hadeed_Target_Feature_t *target) {
    if (target == NULL) return;
    uint8_t payload[9] = {0};
    put_i16_be(&payload[0], (int16_t)target->peak_amplitude);
    payload[2] = (uint8_t)(target->estimated_depth_cm > 255.0f ? 255u : target->estimated_depth_cm);
    payload[3] = target->confidence_score;
    payload[4] = target->target_type;
    (void)send_frame(HADEED_PROTOCOL_MSG_TARGET_UPDATE, payload, sizeof(payload));
}

void Hadeed_Comm_Send_Voice_Trigger(uint16_t voice_packet) {
    uint8_t payload[2] = {(uint8_t)(voice_packet >> 8), (uint8_t)voice_packet};
    (void)send_frame(HADEED_PROTOCOL_MSG_VOICE_TRIGGER, payload, sizeof(payload));
}

void Hadeed_Comm_Send_3D_Frame(float mag_val, float pi_val, float x, float y) {
    uint8_t payload[8];
    put_i16_be(&payload[0], (int16_t)mag_val);
    put_i16_be(&payload[2], (int16_t)pi_val);
    put_i16_be(&payload[4], (int16_t)(x * 100.0f));
    put_i16_be(&payload[6], (int16_t)(y * 100.0f));
    (void)send_frame(HADEED_PROTOCOL_MSG_3D_FRAME, payload, sizeof(payload));
}

/* Insecure fixed challenge/XOR authentication was intentionally removed.
 * This remains disabled until BLE Secure Connections and key provisioning are
 * implemented with a hardware-backed secret. */
bool Hadeed_Comm_Perform_Security_Handshake(void) {
    return false;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        /* Command decoding will be added after the command schema is frozen. */
    }
}
