#include "k210_link.h"
#include "hadeed_sync.h"
#include "hadeed_protocol.h"
#include "main.h"
#include <stdbool.h>
#include <string.h>

static uint8_t k210_rx_buffer[HADEED_PROTOCOL_MAX_FRAME_SIZE];
static uint8_t k210_tx_buffer[HADEED_PROTOCOL_MAX_FRAME_SIZE];
static Hadeed_K210_Response_t last_ai_result;
static volatile bool new_data_ready;
static volatile uint32_t last_rx_tick;

void H7_K210_Init(void) {
    memset(&last_ai_result, 0, sizeof(last_ai_result));
    new_data_ready = false;
    last_rx_tick = HAL_GetTick();
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
        Hadeed_K210_Response_t decoded;
        if (Hadeed_Sync_DecodeVisionFrame(k210_rx_buffer, size, &decoded)) {
            last_ai_result = decoded;
            last_rx_tick = HAL_GetTick();
            new_data_ready = true;
            extern osThreadId_t BrainTaskHandle;
            if (BrainTaskHandle != NULL) (void)osThreadFlagsSet(BrainTaskHandle, 0x04u);
        }
        (void)HAL_UARTEx_ReceiveToIdle_DMA(&huart2, k210_rx_buffer, sizeof(k210_rx_buffer));
    }
}

bool H7_K210_Is_Link_Alive(void) {
    return (HAL_GetTick() - last_rx_tick) < 2000u;
}
