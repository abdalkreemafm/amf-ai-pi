/**
  ******************************************************************************
  * @file           : hadeed_empathy.c
  * @brief          : Emotional AI & User Pacing Engine
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * This engine manages the AI Companion's personality and voice prompts.
  * It prevents repetitive annoyance and selects supportive Arabic phrases.
  ******************************************************************************
  */

#include "hadeed_empathy.h"
#include "hadeed_logic.h"
#include "hadeed_config.h"
#include "main.h"

/* --- 1. PRIVATE GLOBAL STRUCTURE --- */
static Hadeed_Empathy_Engine_t ai_companion;

/**
 * @brief تهيئة محرك التعاطف وضبط الشخصية الأولية
 */
void Hadeed_Empathy_Init(void) {
    ai_companion.personality = EMPATHY_VERBOSE; // يبدأ ثرثاراً لتوجيه المستخدم في البداية
    ai_companion.last_talk_timestamp = 0;
    ai_companion.targets_found_today = 0;
    ai_companion.current_variation_idx = 0;
}

/* --- 2. INTERNAL UTILS --- */

/**
 * @brief فحص "فترة التبريد" الصوتي لمنع الثرثرة المزعجة
 * @return true إذا كان مسموحاً للجهاز بالتحدث الآن
 */
static bool Is_Cooldown_Expired(uint32_t cooldown_ms) {
    if ((HAL_GetTick() - ai_companion.last_talk_timestamp) > cooldown_ms) {
        return true;
    }
    return false;
}

/**
 * @brief تحديث مستوى الشخصية بناءً على زمن التشغيل
 * يقلل الثرثرة تلقائياً بعد 10 دقائق من العمل (Zen Mode)
 */
static void Auto_Adjust_Personality(void) {
    uint32_t uptime_sec = HAL_GetTick() / 1000;
    
    if (uptime_sec > EMOTIONAL_SILENCE_DELAY_SEC) {
        ai_companion.personality = EMPATHY_ZEN;
    } else if (uptime_sec > 300) { // بعد 5 دقائق
        ai_companion.personality = EMPATHY_BALANCED;
    }
}

/* --- 3. THE HEART OF EMPATHY (إصدار الأوامر الصوتية الذكية) --- */

/**
 * @brief اختيار وإرسال الكود الصوتي العاطفي المناسب
 * @param event_code كود الحدث (ذهب، سرعة، صخور، إلخ)
 */
void Hadeed_Empathy_Speak(uint8_t event_code) {
    uint8_t variation = Get_Empathy_Variation(event_code);
    uint32_t cooldown = 5000; // الافتراضي 5 ثوانٍ بين الجمل

    // تعديل الكولدوان بناءً على نوع الحدث والشخصية
    if (event_code == VOICE_CMD_SLOW_DOWN) cooldown = 15000; // لا تكرر "تمهل" كثيراً لكي لا يغضب
    if (event_code == VOICE_CMD_GOLD_HIGH) cooldown = 2000;  // الذهب مسموح بتكرار تنبيهه

    // إذا كان الجهاز في طور Zen، لا يتحدث إلا في الأمور الخطيرة أو الذهب
    if (ai_companion.personality == EMPATHY_ZEN) {
        if (event_code != VOICE_CMD_GOLD_HIGH && event_code != VOICE_CMD_CAVITY_FOUND) {
            return; 
        }
    }

    // فحص هل الوقت مناسب للتحدث؟
    if (Is_Cooldown_Expired(cooldown)) {
        
        // بناء الباكت الموجه للبلوتوث: (كود الحدث + رقم الجملة المتغيرة)
        // سيقوم التطبيق باستلام variation واختيار جملة مختلفة من المصفوفة
        uint16_t final_voice_packet = (event_code << 8) | variation;
        
        Hadeed_Comm_Send_Voice_Trigger(final_voice_packet);
        
        // تحديث سجل المحادثة
        ai_companion.last_talk_timestamp = HAL_GetTick();
        Auto_Adjust_Personality();
    }
}

/* --- 4. BEHAVIORAL MONITORING (مراقبة سلوك المنقب) --- */

/**
 * @brief مراقبة سرعة الأرجحة وتوجيه المنقب عاطفياً
 * @param current_speed السرعة من الـ IMU
 */
void Hadeed_Empathy_Analyze_Pacing(float current_speed) {
    if (current_speed > SWING_SPEED_ALERT_LIMIT) {
        // بدلاً من أمر جاف، نرسل تنبيه "تمهل يا صديقي"
        Hadeed_Empathy_Speak(VOICE_CMD_SLOW_DOWN);
    }
}

/**
 * @brief تشجيع المنقب عند اكتشاف أهداف متكررة
 */
void Hadeed_Empathy_Register_Find(void) {
    ai_companion.targets_found_today++;
    
    // إذا وجد 5 أهداف اليوم، قل له جملة تحفيزية خاصة
    if (ai_companion.targets_found_today == 5) {
        // إرسال كود "يوم محظوظ" (مخزن في التطبيق)
        Hadeed_Comm_Send_Voice_Trigger(0xCC01); 
    }
}

/**
 * @brief التعامل مع الأخطاء بصدق وشفافية عاطفية
 * بدلاً من الصمت عند الخطأ، يخبر الجهاز المنقب بما حدث
 */
void Hadeed_Empathy_Explain_Error(uint8_t error_type) {
    switch(error_type) {
        case 0x01: // خطأ مغناطيسية
            // "يا صديقي، هناك تشويش كهرومغناطيسي قوي، سأحاول معالجته صمتاً."
            Hadeed_Comm_Send_Voice_Trigger(0xEE01); 
            break;
        case 0x02: // حرارة عالية
            // "شمس اليوم قوية، سأخفف طاقتي قليلاً لكي لا أتعب، ابقَ معي."
            Hadeed_Comm_Send_Voice_Trigger(0xEE02);
            break;
    }
}
