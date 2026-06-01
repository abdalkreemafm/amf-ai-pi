/**
  ******************************************************************************
  * @file           : k210_link.c
  * @brief          : High-Speed Bridge for AI Vision (H7 <-> K210)
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * Uses UART DMA for non-blocking asynchronous AI inference reception.
  * Synchronizes timestamps between Vision and Pulse Induction data.
  ******************************************************************************
  */

#include "k210_link.h"
#include "hadeed_sync.h"
#include "hadeed_logic.h"
#include "main.h"
#include <string.h>

/* --- 1. PRIVATE VARIABLES --- */
static uint8_t k210_rx_buffer[sizeof(Hadeed_K210_Response_t) + 2]; // مخزن الاستلام
static Hadeed_K210_Response_t last_ai_result;
static bool new_data_ready = false;

/* --- 2. INTERNAL UTILS --- */

/**
 * @brief حساب فحص السلامة (Checksum) البسيط للحزم الصادرة
 */
static uint16_t Compute_Simple_Checksum(uint8_t *data, uint16_t len) {
    uint16_t sum = 0;
    for (uint16_t i = 0; i < len; i++) sum += data[i];
    return sum;
}

/* --- 3. MAIN INTERFACE FUNCTIONS --- */

/**
 * @brief تهيئة جسر التواصل وتفعيل استلام البيانات عبر الـ DMA
 */
void H7_K210_Init(void) {
    // تصفير الهياكل
    memset(&last_ai_result, 0, sizeof(Hadeed_K210_Response_t));
    
    // بدء الاستلام المستمر عبر الـ DMA
    // الحساس يرسل النتيجة فور انتهاء التحليل (Inference)
    HAL_UART_Receive_DMA(&huart2, k210_rx_buffer, sizeof(k210_rx_buffer));
}

/**
 * @brief إرسال حزمة المزامنة (Timestamp) لمعالج الرؤية
 * @param packet الحزمة المجهزة في hadeed_sync.c
 */
HAL_StatusTypeDef H7_K210_SendPacket(Hadeed_Sync_Packet_t *packet) {
    packet->checksum = Compute_Simple_Checksum((uint8_t*)packet, sizeof(Hadeed_Sync_Packet_t) - 2);
    
    // الإرسال سريع جداً ولا يعطل المهام الأخرى
    return HAL_UART_Transmit_IT(&huart2, (uint8_t*)packet, sizeof(Hadeed_Sync_Packet_t));
}

/**
 * @brief استلام ومعالجة الرد القادم من معالج الرؤية
 * @param response مؤشر للهيكل المراد تعبئته بالنتيجة
 * @return HAL_OK إذا تم استلام تصنيف جديد بنجاح
 */
HAL_StatusTypeDef H7_K210_ReceiveResponse(Hadeed_K210_Response_t *response) {
    if (new_data_ready) {
        memcpy(response, &last_ai_result, sizeof(Hadeed_K210_Response_t));
        new_data_ready = false;
        return HAL_OK;
    }
    return HAL_BUSY;
}

/* --- 4. INTERRUPT CALLBACKS (رد الفعل السريع) --- */

/**
 * @brief هذه الدالة تستدعى تلقائياً من المعالج عند اكتمال استلام باكت من K210
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) { // منفذ الـ K210
        
        // التحقق من رمز البداية (SOF) لضمان عدم تداخل البيانات
        if (k210_rx_buffer[0] == HADEED_SYNC_SOF) {
            
            // نقل البيانات من المخزن الخام للهيكل المنظم
            // نتخطى أول بايت (SOF)
            memcpy(&last_ai_result, &k210_rx_buffer[1], sizeof(Hadeed_K210_Response_t));
            
            new_data_ready = true;
            
            // إرسال إشارة لمهمة الذكاء الاصطناعي (BrainTask) بأن نتيجة الرؤية جاهزة
            extern osThreadId_t BrainTaskHandle;
            if (BrainTaskHandle != NULL) {
                osThreadFlagsSet(BrainTaskHandle, 0x04); // Flag 0x04: Vision Ready
            }
        }
        
        // إعادة تشغيل الـ DMA للاستلام القادم
        HAL_UART_Receive_DMA(&huart2, k210_rx_buffer, sizeof(k210_rx_buffer));
    }
}

/**
 * @brief فحص جودة الاتصال مع معالج الرؤية
 */
bool H7_K210_Is_Link_Alive(void) {
    static uint32_t last_heartbeat = 0;
    
    // إذا لم نستلم أي نتيجة تصنيف لمدة أكثر من 2 ثانية، الرؤية معطلة
    if (new_data_ready) {
        last_heartbeat = HAL_GetTick();
    }
    
    return (HAL_GetTick() - last_heartbeat < 2000);
}
