/**
  ******************************************************************************
  * @file           : hadeed_logic.h
  * @brief          : The Decision Logic & Voice Protocol for Hadeed V7.0
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * This file defines target classifications, system states, and the 
  * "Sentient Companion" BLE communication protocol.
  ******************************************************************************
  */

#ifndef HADEED_LOGIC_H
#define HADEED_LOGIC_H

#include <stdint.h>

/* --- 1. TARGET CLASSIFICATION (هوية الأهداف) --- */
typedef enum {
    TARGET_ID_NONE          = 0x00,
    TARGET_ID_TREASURE_GOLD = 0x01,  // ذهب تكنيزي (عملات، سبائك) - انحدار ناعم
    TARGET_ID_NATURAL_GOLD  = 0x02,  // ذهب طبيعي (عروق، شذر) - انحدار خشن
    TARGET_ID_SILVER        = 0x03,  // فضة
    TARGET_ID_CAVITY        = 0x04,  // فراغ / ران صخري (انقطاع مغناطيسي)
    TARGET_ID_FERROUS       = 0x05,  // حديد عادي
    TARGET_ID_RUSTY_IRON    = 0x06,  // حديد قديم مصدأ (تأثير الهالة)
    TARGET_ID_HOT_ROCK      = 0x07,  // صخور بازلتية مشعة
    TARGET_ID_UNKNOWN       = 0xFF   // إشارة غامضة تحتاج حفر للتعلم
} Hadeed_Target_Type_t;

/* --- 2. SENTIENT SYSTEM STATES (أطوار الوعي بالجهاز) --- */
typedef enum {
    STATE_BOOTING,           // مرحلة الفحص الذاتي والتعرف على الكويل
    STATE_ENVIRONMENT_SCAN,  // أول 5 ثوانٍ: دراسة ضجيج الأرض والجو
    STATE_STEALTH_SCAN,      // المسح الصامت (الطور الشبح)
    STATE_ALERT_SENSING,     // التقاط "ذيل" إشارة ضعيفة
    STATE_DEEP_INVESTIGATE,  // طور التحقيق (عند ضغط زر الـ Pinpoint)
    STATE_EMERGENCY_RECOVERY,// طور التعافي من خطأ حساس
    STATE_POWER_ADVISOR      // طور الحفاظ على البطارية
} Hadeed_System_State_t;

/* --- 3. BLE VOICE COMPANION PROTOCOL (بروتوكول المرافق الصوتي) --- */
/**
 * هذه الأكواد يتم إرسالها للتطبيق لتشغيل الجمل العاطفية والميدانية.
 * التطبيق يمتلك مصفوفة جمل لكل كود (Dynamic Empathy).
 */
#define VOICE_CMD_WELCOME        0xA0  // "أهلاً بك يا صديقي، أنا جاهز.."
#define VOICE_CMD_SOIL_READY     0xA1  // "تم ضبط التربة، يمكنك البدء الآن."
#define VOICE_CMD_GOLD_HIGH      0xB0  // "رصدتُ هدفاً ثميناً جداً، تمهل!"
#define VOICE_CMD_GOLD_DEEP      0xB1  // "إشارة ذهب عميقة، نحتاج مسحاً هادئاً."
#define VOICE_CMD_CAVITY_FOUND   0xC0  // "انتباه، يوجد ران أو تجويف في الصخر."
#define VOICE_CMD_HOT_ROCK_SKIP  0xD0  // "هذه مجرد صخرة بازلت، تجاوزها."
#define VOICE_CMD_SLOW_DOWN      0xE0  // "تمهل رجاءً لكي لا يذهب عملنا سدى."
#define VOICE_CMD_COIL_HIGH      0xE1  // "أخفض ملف البحث قليلاً نحو الأرض."
#define VOICE_CMD_BATT_LOW       0xF0  // "البطارية منخفضة، سأنتقل للوضع الشبح."
#define VOICE_CMD_APP_OPEN_3D    0xF5  // "افتح التطبيق الآن لرؤية المجسم 3D."

/* --- 4. DECISION THRESHOLDS (عتبات القرار) --- */
#define CONFIDENCE_LEVEL_HIGH    90    // نسبة الثقة المطلوبة لإعلان "ذهب"
#define CONFIDENCE_LEVEL_MEDIUM  70
#define MIN_SIGNAL_TO_NOISE      12    // الحد الأدنى لإيقاظ الحساسات (SNR)
#define MAX_SWING_SPEED_LIMIT    1.5f  // (متر/ثانية) أقصى سرعة أرجحة مسموحة

/* --- 5. HARDWARE ADDRESSES (عناوين الهاردوير الثابتة) --- */
#define RM3100_I2C_ADDR          0x20 << 1
#define ADS1256_SPI_ADDR         0x01
#define K210_UART_BAUDRATE       115200
#define BLE_UART_BAUDRATE        921600 // سرعة عالية لمنع تأخير الصوت

/* --- 6. MEMORY REPLACEMENT TAGS (للتحديثات الهوائية) --- */
#define HADEED_FW_VER            0x0700 // v7.0.0
#define HADEED_MODEL_ID          0x55AA // بصمة أمان الجهاز

#endif /* HADEED_LOGIC_H */
