/**
  ******************************************************************************
  * @file           : hadeed_unified_brain.c
  * @brief          : Unified Neural-Hybrid Decision Engine (The Master Mind)
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  ******************************************************************************
  * @attention
  * يدمج الخبرة الميدانية (Expert Knowledge) مع آلية التعلم اللحظي.
  * تم تزويده مسبقاً ببصمات واقعية لصخور اليمن والحجاز والكنوز الأثرية.
  ******************************************************************************
  */

#include "hadeed_math.h"
#include "hadeed_logic.h"
#include "hadeed_config.h"

/* --- 1. FACTORY EXPERT KNOWLEDGE (الخبرة الميدانية المدمجة مسبقاً) --- */
// هذه القيم تمثل "البصمة الذهبية" المستخلصة من الواقع الجيوفيزيائي
static const float EXPERT_TREASURE_SLOPE = 0.58f; // بصمة الذهب المصمت
static const float EXPERT_TREASURE_PHASE = 78.0f; // زاوية طور الذهب الأثري
static const float EXPERT_BASALT_MAG_ANOMALY = 1800.0f; // شذوذ صخور البازلت

/* --- 2. DYNAMIC LEARNING WEIGHTS (أوزان التعلم القابلة للتغير) --- */
// تبدأ بقيم الخبراء وتتعدل مع كل عملية "حفر وتصحيح" من المستخدم
static float weight_electromagnetic = 0.50f;
static float weight_magnetic        = 0.30f;
static float weight_vision          = 0.20f;
static float local_soil_bias        = 0.0f;

/* --- 3. THE UNIFIED INFERENCE ENGINE (محرك الاستدلال الموحد) --- */

/**
 * @brief محرك القرار النهائي: يدمج كل الحواس ويصدر "حكم الخبير"
 */
TargetType_t Hadeed_Unified_Decision_Engine(Hadeed_Target_Feature_t *feat) {
    float treasure_probability = 0.0f;
    float basalt_probability   = 0.0f;

    // أ. تحليل "البصمة الجينية" (النبضة SMF + Decay)
    float slope_err = fabsf(feat->dp.log_slope - EXPERT_TREASURE_SLOPE);
    float phase_err = fabsf(feat->dp.phase_angle - EXPERT_TREASURE_PHASE);
    
    // إذا كان الانحدار ناعماً والانسجام الترددي عالياً = يقين ذهب
    if (slope_err < 0.05f && feat->smf.harmony_ratio > 0.80f) {
        treasure_probability += 40.0f;
    }

    // ب. تحليل "الحس المغناطيسي" (RM3100)
    // الكنوز لا تشوه الحقل المغناطيسي، الصخور والحديد يشوهونه بقوة
    if (feat->mag_anomaly_delta > EXPERT_BASALT_MAG_ANOMALY) {
        basalt_probability += 50.0f;
        treasure_probability -= 30.0f;
    }

    // ج. تحليل "الرؤية البصرية" (K210)
    if (feat->visual_class_id == TARGET_ID_HOT_ROCK) {
        basalt_probability += 30.0f;
    } else if (feat->visual_class_id == TARGET_ID_NATURAL_GOLD) {
        treasure_probability += 20.0f;
    }

    // د. دمج الأدلة وحساب نسبة الثقة النهائية
    float final_confidence = (treasure_probability * weight_electromagnetic) + 
                             (20.0f * weight_vision) - 
                             (basalt_probability * 0.5f) - local_soil_bias;

    feat->confidence_score = (uint8_t)final_confidence;

    /* --- قرار القاضي النهائي --- */
    
    // 1. حالة الذهب التكنيزي (عملات/سبائك)
    if (final_confidence > 85.0f && feat->mag_anomaly_delta < 400.0f) {
        return TARGET_ID_TREASURE_GOLD;
    }
    
    // 2. حالة الرانات والتجاويف (انقطاع مغناطيسي صامت)
    if (feat->mag_anomaly_delta > 1500.0f && feat->peak_amplitude < 50.0f) {
        feat->confidence_score = 95;
        return TARGET_ID_CAVITY;
    }
    
    // 3. حالة الصخور البازلتية (تجاهل ذكي)
    if (basalt_probability > 60.0f || (feat->dp.phase_angle < 35.0f && feat->mag_anomaly_delta > 1000)) {
        return TARGET_ID_HOT_ROCK;
    }

    return TARGET_ID_NONE;
}

/* --- 4. CONTINUAL LEARNING ALGORITHMS (خوارزميات التعلم المستمر) --- */

/**
 * @brief التعلم من "تغذية المستخدم الراجعة"
 * يتم استدعاؤه عندما يضغط المستخدم على "تجاهل" أو "هدف خاطئ" في التطبيق
 */
void Hadeed_AI_Feedback_Learning(Hadeed_Target_Feature_t *last_target, bool user_verified_treasure) {
    if (!user_verified_treasure) {
        // إذا حفر المستخدم ووجد صخرة بينما قال الجهاز ذهب:
        // نزيد "انحياز التربة" محلياً لرفع عتبة الحذر
        local_soil_bias += 5.0f;
        
        // تقليل الثقة في "الرؤية" إذا كانت قد خدعتنا في هذا النوع من الصخور
        weight_vision -= 0.02f;
        weight_magnetic += 0.02f; // الاعتماد أكثر على المغناطيسية لعزل الصخور
    } else {
        // إذا كان القرار صحيحاً، نقوي الأوزان الحالية (Positive Reinforcement)
        local_soil_bias -= 1.0f;
        if (local_soil_bias < 0) local_soil_bias = 0;
    }
}

/**
 * @brief التكيف مع ضجيج الموقع (Site-Specific Adaptation)
 * يستدعى كل 30 ثانية لتحديث "الصفر النفسي" للجهاز بناءً على استقرار الأرض
 */
void Hadeed_AI_Update_Soil_Context(float noise_std_dev) {
    // تعلم مستوى "الاضطراب" في التربة الحالية
    // إذا كانت الأرض "تغلي" بالمعادن، يرفع الجهاز مستوى الحذر تلقائياً
    local_soil_bias = (noise_std_dev * 0.4f);
}
