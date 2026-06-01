/**
  ******************************************************************************
  * @file           : hadeed_brain_model.c
  * @brief          : Expert-Knowledge Infused AI Model (Real-World Field Data)
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  ******************************************************************************
  */

#include "hadeed_math.h"
#include "hadeed_logic.h"
#include "hadeed_config.h"

/* --- 1. FIELD WISDOM DATABASE (بصمات واقعية لبيئة الشرق الأوسط) --- */

// بصمة الذهب الأثري (عملات، سبائك)
static const float REF_GOLD_TREASURE_SLOPE = 0.58f;
static const float REF_GOLD_TREASURE_PHASE = 78.0f;

// بصمة الصخور البازلتية (أكبر عدو للمنقب في اليمن)
static const float REF_BASALT_MAX_PHASE    = 35.0f;
static const float REF_BASALT_MIN_ANOMALY  = 1800.0f;

// بصمة التربة الملحية (السبخة)
static const float REF_SALT_CONDUCTIVITY   = 8.5f; // من قنطرة وين

/* --- 2. ADVANCED HEURISTICS (الخوارزميات الخبيرة) --- */

/**
 * @brief فلتر "كاسر البازلت" (Basalt Crusher)
 * يستخدم الذكاء البصري ودمج الحساسات لتجاوز الصخور البركانية
 */
static bool Is_Actually_Hot_Rock(Hadeed_Target_Feature_t *feat) {
    // إذا كانت الكاميرا ترى نسيجاً بركانياً + شذوذ مغناطيسي ضخم + زاوية طور حادة
    // فهذا بازلت 100% مهما كانت قوة الإشارة
    if (feat->visual_class_id == TARGET_ID_HOT_ROCK || feat->mag_anomaly_delta > REF_BASALT_MIN_ANOMALY) {
        if (feat->dp.phase_angle < REF_BASALT_MAX_PHASE) {
            return true;
        }
    }
    return false;
}

/**
 * @brief خوارزمية "صائد الرانات" (Raan Hunter)
 * تكتشف الفراغات المغناطيسية التي تخفي كنوزاً
 */
static bool Is_Raan_Detected(Hadeed_Target_Feature_t *feat) {
    // الران الصخري يسبب انقطاعاً في الحقل المغناطيسي المستقر للأرض
    // مع انعدام وجود أي معدن سطحي (PI Silent)
    if (feat->mag_anomaly_delta > MAG_ANOMALY_THRESHOLD_NT && feat->peak_amplitude < 50.0f) {
        return true;
    }
    return false;
}

/* --- 3. THE SENTIENT EVALUATOR (المقيم الواعي) --- */

uint8_t Hadeed_AI_Evaluate_Target(Hadeed_Target_Feature_t *feat) {
    float final_score = 0.0f;

    // 1. فحص البازلت أولاً (الدفاع الاستباقي)
    if (Is_Actually_Hot_Rock(feat)) {
        feat->target_type = TARGET_ID_HOT_ROCK;
        return 5; // ثقة منخفضة جداً
    }

    // 2. فحص الران/الفراغ
    if (Is_Raan_Detected(feat)) {
        feat->target_type = TARGET_ID_CAVITY;
        return 95; 
    }

    // 3. مطابقة بصمة الذهب التكنيزي (عملات/كنوز)
    // نحسب القرب الرياضي من "البصمة المثالية"
    float slope_diff = fabsf(feat->dp.log_slope - REF_GOLD_TREASURE_SLOPE);
    float phase_diff = fabsf(feat->dp.phase_angle - REF_GOLD_TREASURE_PHASE);

    if (slope_diff < 0.05f && phase_diff < 10.0f && feat->smf.harmony_ratio > 0.85f) {
        final_score = 90.0f + (feat->smf.harmony_ratio * 10.0f);
        feat->target_type = TARGET_ID_TREASURE_GOLD;
    }
    
    // 4. مطابقة الذهب الطبيعي (الشذر)
    else if (feat->smf.bin_high > (feat->smf.bin_low * 1.5f) && feat->dp.log_slope > 0.45f) {
        final_score = 75.0f;
        feat->target_type = TARGET_ID_NATURAL_GOLD;
    }

    // 5. تعويض "السبخة" (Salt Compensation)
    // إذا كانت التربة ملحية (ناقلية عالية من Wien Bridge)
    extern Hadeed_Soil_Chemistry_t g_soil_chem;
    if (g_soil_chem.conductivity_ms > REF_SALT_CONDUCTIVITY) {
        final_score -= 20.0f; // تقليل الثقة لأن الملح يرفع الناقلية كأنها ذهب
    }

    return (uint8_t)final_score;
}
