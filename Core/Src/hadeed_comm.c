/**
  ******************************************************************************
  * @file           : hadeed_comm.c
  * @brief          : Encrypted Communication & BLE Protocol Engine
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * Manages encrypted handshakes with the mobile app and high-speed data 
  * streaming for 3D visualization. Uses DMA for non-blocking UART transfer.
  ******************************************************************************
  */

#include "hadeed_math.h"
#include "hadeed_logic.h"
#include "hadeed_config.h"
#include "main.h"
#include <string.h>

/* --- 1. PRIVATE CONSTANTS & PACKET STRUCTURE --- */
#define COMM_START_BYTE          0xAA  // رمز بداية الحزمة
#define COMM_VERSION             0x07  // نسخة البروتوكول v7
#define AES_KEY_HINT             0x5F  // مفتاح تشفير بسيط (يُستبدل بـ AES في النسخة النهائية)

#pragma pack(push, 1)
typedef struct {
    uint8_t  start_byte;
    uint8_t  proto_ver;
    uint8_t  packet_type;   // 0x10: Target, 0x20: Voice, 0x30: 3D Data
    uint8_t  payload[12];   // البيانات الفعلية
    uint16_t crc16;         // فحص السلامة
} Hadeed_Packet_t;
#pragma pack(pop)

/* --- 2. INTERNAL UTILS --- */

/**
 * @brief حساب الـ CRC16 لضمان عدم ضياع أي بت في الجو
 */
static uint16_t Compute_CRC16(uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}

/**
 * @brief تشفير الحزمة (XOR Encryption) لمنع التطبيقات الخارجية من قراءة البيانات
 */
static void Hadeed_Encrypt_Payload(uint8_t *payload, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        payload[i] ^= (AES_KEY_HINT + i);
    }
}

/* --- 3. MAIN COMMUNICATION INTERFACE --- */

/**
 * @brief إرسال حزمة بيانات متكاملة للتطبيق عبر الـ DMA
 */
static void Hadeed_Comm_Send_Raw(Hadeed_Packet_t *pkt) {
    pkt->crc16 = Compute_CRC16((uint8_t*)pkt, sizeof(Hadeed_Packet_t) - 2);
    // إرسال عبر UART الموصول بموديول البلوتوث (ESP32-C3)
    HAL_UART_Transmit_DMA(&huart1, (uint8_t*)pkt, sizeof(Hadeed_Packet_t));
}

/**
 * @brief إرسال تحديث الهدف (الذهب، الفراغ، العمق) للشاشة والتطبيق
 */
void Hadeed_Comm_Send_Target_Update(Hadeed_Target_Feature_t *target) {
    Hadeed_Packet_t pkt;
    pkt.start_byte  = COMM_START_BYTE;
    pkt.proto_ver   = COMM_VERSION;
    pkt.packet_type = 0x10; // Target Update

    // تعبئة البيانات (تحويل Floating point لـ Bytes مضغوطة)
    int16_t amp   = (int16_t)(target->peak_amplitude);
    int8_t  depth = (uint8_t)(target->estimated_depth_cm);
    int8_t  conf  = (uint8_t)(target->confidence_score);
    uint8_t type  = target->target_type;

    memcpy(&pkt.payload[0], &amp, 2);
    pkt.payload[2] = depth;
    pkt.payload[3] = conf;
    pkt.payload[4] = type;

    Hadeed_Encrypt_Payload(pkt.payload, 12);
    Hadeed_Comm_Send_Raw(&pkt);
}

/**
 * @brief إرسال "الزناد الصوتي" (Voice Trigger) للمرافق الذكي
 * @param voice_packet يحتوي على (Event Code + Variation ID)
 */
void Hadeed_Comm_Send_Voice_Trigger(uint16_t voice_packet) {
    Hadeed_Packet_t pkt;
    pkt.start_byte  = COMM_START_BYTE;
    pkt.proto_ver   = COMM_VERSION;
    pkt.packet_type = 0x20; // Voice Command

    memcpy(pkt.payload, &voice_packet, 2);
    
    // تصفير بقية الـ payload
    memset(&pkt.payload[2], 0, 10);

    Hadeed_Encrypt_Payload(pkt.payload, 12);
    Hadeed_Comm_Send_Raw(&pkt);
}

/**
 * @brief إرسال بيانات المسح الميداني للرسم ثلاثي الأبعاد (3D Visualization)
 */
void Hadeed_Comm_Send_3D_Frame(float mag_val, float pi_val, float x, float y) {
    Hadeed_Packet_t pkt;
    pkt.start_byte  = COMM_START_BYTE;
    pkt.proto_ver   = COMM_VERSION;
    pkt.packet_type = 0x30; // 3D Spatial Frame

    // تحويل البيانات لإحداثيات فراغية يفهمها محرك Unity/Flutter
    int16_t m = (int16_t)mag_val;
    int16_t p = (int16_t)pi_val;
    int16_t posX = (int16_t)(x * 100);
    int16_t posY = (int16_t)(y * 100);

    memcpy(&pkt.payload[0], &m, 2);
    memcpy(&pkt.payload[2], &p, 2);
    memcpy(&pkt.payload[4], &posX, 2);
    memcpy(&pkt.payload[6], &posY, 2);

    Hadeed_Encrypt_Payload(pkt.payload, 12);
    Hadeed_Comm_Send_Raw(&pkt);
}

/* --- 4. SECURITY HANDSHAKE (المصافحة الأمنية) --- */

/**
 * @brief التحقق من هوية التطبيق (منع البرامج المزيفة من الاتصال)
 */
bool Hadeed_Comm_Perform_Security_Handshake(void) {
    uint8_t challenge[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t response[4];

    // إرسال تحدي للتطبيق
    HAL_UART_Transmit(&huart1, challenge, 4, 100);

    // انتظار الرد المشفر من الموبايل
    if (HAL_UART_Receive(&huart1, response, 4, 1000) == HAL_OK) {
        // التحقق من مفتاح المصنع (مثال بسيط)
        if (response[0] == (challenge[0] ^ HADEED_MODEL_ID)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief استقبال الأوامر من التطبيق (مثلاً: تغيير نمط البحث)
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        // هنا يتم استقبال أوامر المستخدم من الموبايل (مثل طلب المعايرة)
        // سيتم معالجتها في ملف hadeed_state_machine.c
    }
}
