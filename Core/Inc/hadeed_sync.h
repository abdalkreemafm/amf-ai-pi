/**
  ******************************************************************************
  * @file           : hadeed_sync.h
  * @brief          : Inter-Processor Clock Synchronization (H7 <-> K210)
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * Ensures temporal alignment between AI Vision and Electromagnetic data.
  * Uses a high-resolution heartbeat to prevent data drift.
  ******************************************************************************
  */

#ifndef HADEED_SYNC_H
#define HADEED_SYNC_H

#include <stdint.h>
#include <stdbool.h>

/* --- 1. SYNC PROTOCOL DEFINITIONS --- */
#define HADEED_SYNC_SOF          0x5A        // رمز بداية الإطار (Start of Frame)
#define MAX_ALLOWED_DRIFT_MS     5           // أقصى تفاوت مسموح به (5 ملي ثانية)
#define HEARTBEAT_INTERVAL_MS    100         // إرسال نبضة مزامنة كل 100 ملي ثانية

/* --- 2. SYNC STATES (حالات التزامن) --- */
typedef enum {
    SYNC_STATE_DISCONNECTED, // لا يوجد اتصال مع معالج الرؤية
    SYNC_STATE_HANDSHAKING,  // جاري تبادل مفاتيح التوقيت
    SYNC_STATE_LOCKED,       // العقول متزامنة تماماً
    SYNC_STATE_DRIFT_WARN,   // بدأ التوقيت بالانحراف
    SYNC_STATE_ERROR         // خطأ فادح في البيانات
} Hadeed_Sync_State_t;

/* --- 3. DATA PACKET STRUCTURES --- */

/**
 * @brief حزمة بيانات المزامنة (The Sync Packet)
 * تُرسل من STM32H7 (الماستر) إلى K210 (العبد)
 */
#pragma pack(push, 1)
typedef struct {
    uint8_t  sof;             // رمز البداية 0x5A
    uint32_t master_timestamp;// توقيت المعالج الرئيسي (ms)
    uint32_t sequence_id;     // رقم التسلسل لمنع تكرار البيانات
    uint8_t  system_state;    // الحالة الحالية للجهاز (من hadeed_logic.h)
    uint16_t checksum;        // لضمان سلامة الحزمة (CRC16)
} Hadeed_Sync_Packet_t;
#pragma pack(pop)

/**
 * @brief تقرير العودة من معالج الرؤية (K210 Response)
 */
typedef struct {
    uint32_t slave_timestamp; // متى استلم K210 الحزمة بتوقيته الخاص
    uint8_t  detected_class;  // نتيجة تصنيف الكاميرا للتربة
    float    inference_time;  // كم استغرق الذكاء البصري للتحليل
} Hadeed_K210_Response_t;

/* --- 4. THE SYNC MANAGER OBJECT --- */

/**
 * @brief مدير التزامن المركزي
 */
typedef struct {
    Hadeed_Sync_State_t state;
    uint32_t last_sync_time;
    uint32_t round_trip_latency_ms; // زمن ذهاب وعودة الإشارة
    int32_t  clock_offset;          // الفرق الزمني المحسوب بين المعالجين
    uint32_t missed_heartbeats;
} Hadeed_Sync_Manager_t;

/* --- 5. SYNC INTERFACE FUNCTIONS --- */

/**
 * @brief تهيئة بروتوكول التزامن
 */
void Hadeed_Sync_Init(Hadeed_Sync_Manager_t *sm);

/**
 * @brief إنشاء حزمة مزامنة لإرسالها لـ K210
 * @param packet مؤشر للحزمة المراد تعبئتها
 */
void Hadeed_Sync_PreparePacket(Hadeed_Sync_Packet_t *packet);

/**
 * @brief معالجة الرد القادم من K210 وحساب التأخير (Latency)
 * @param sm مدير التزامن
 * @param response الرد المستلم
 */
void Hadeed_Sync_ProcessResponse(Hadeed_Sync_Manager_t *sm, Hadeed_K210_Response_t *response);

/**
 * @brief التحقق من سلامة التزامن
 * @return true إذا كانت "العقول" متطابقة زمنياً
 */
bool Hadeed_Sync_IsLocked(Hadeed_Sync_Manager_t *sm);

#endif /* HADEED_SYNC_H */
