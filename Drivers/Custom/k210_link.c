#include "k210_link.h"
#include "hadeed_sync.h"
#include "hadeed_protocol.h"
#include "hadeed_framer.h"
#include "main.h"
#include "cmsis_os.h"
#include <stdbool.h>
#include <string.h>

static uint8_t k210_rx_buffer[HADEED_PROTOCOL_MAX_FRAME_SIZE];
static uint8_t k210_tx_buffer[HADEED_PROTOCOL_MAX_FRAME_SIZE];
static Hadeed_K210_Response_t last_ai_result;
static volatile bool new_data_ready;
static volatile uint32_t last_rx_tick;
static Hadeed_Framer_t k210_framer;

static void on_k210_frame(const Hadeed_Protocol_Frame_t *frame, void *context) {
    (void)context;
    if (frame == NULL || frame->message_type != HADEED_PROTOCOL_MSG_VISION_RESULT ||
        frame->payload_length != 4u) return;
    last_ai_result.slave_timestamp = frame->timestamp_ms;
    last_ai_result.detected_class = frame->payload[0];
    last_ai_result.confidence = frame->payload[1];
    last_ai_result.inference_time_ms = (uint16_t)(((uint16_t)frame->payload[2] << 8) |
                                                  frame->payload[3]);
    last_rx_tick = HAL_GetTick();
    new_data_ready = true;
}

void H7_K210_Init(void) {
    memset(&last_ai_result, 0, sizeof(last_ai_result));
    new_data_ready = false;
    last_rx_tick = HAL_GetTick();
    Hadeed_Framer_Init(&k210_framer);
    (void)HAL_UARTEx_ReceiveToIdle_DMA(&huart2, k210_rx_buffer, sizeof(k210_rx_buffer));
}

HAL_StatusTypeDef H7_K210_SendPacket(Hadeed_Sync_Packet_t *packet) {
    if (packet == NULL) return HAL_ERROR;
    size_t frame_length = Hadeed_Sync_EncodeFrame(packet, k210_tx_buffer, sizeof(k210_tx_buffer));
    if (frame_length == 0u) return HAL_ERROR;
    return HAL_UART_Transmit_IT(&huart2, k210_tx_buffer, (uint16_t)frame_length);
}

HAL_StatusTypeDef H7_K210_ReceiveResponse(Hadeed_K210_Response_t *response) {
    if (response == NULL) return HAL_ERROR;
    if (!new_data_ready) return HAL_BUSY;
    __disable_irq();
    memcpy(response, &last_ai_result, sizeof(*response));
    new_data_ready = false;
    __enable_irq();
    return HAL_OK;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {
    if (huart->Instance == USART2 && size <= sizeof(k210_rx_buffer)) {
        if (Hadeed_Framer_Feed(&k210_framer, k210_rx_buffer, size,
                               on_k210_frame, NULL) > 0u) {
            extern osThreadId_t BrainTaskHandle;
            if (BrainTaskHandle != NULL) (void)osThreadFlagsSet(BrainTaskHandle, 0x04u);
        }
        (void)HAL_UARTEx_ReceiveToIdle_DMA(&huart2, k210_rx_buffer, sizeof(k210_rx_buffer));
    }
}

bool H7_K210_Is_Link_Alive(void) {
    return (HAL_GetTick() - last_rx_tick) < 2000u;
}
