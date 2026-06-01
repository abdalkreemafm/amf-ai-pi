/**
  ******************************************************************************
  * @file           : hadeed_fusion.c
  * @brief          : Sensor Fusion & Decision Engine (The Court of Justice)
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * Merges PI, SMF, Magnetic, and Vision data to classify targets.
  * Implements Probabilistic Logic for Treasure vs. Natural mineral detection.
  ******************************************************************************
  */

#include "hadeed_math.h"
#include "hadeed_logic.h"
#include "hadeed_config.h"

/* --- 1. INTERNAL LOGIC FUNCTIONS --- */

/**
 * @brief حساب نسبة الثقة بناءً على توافق المعايير الفيزيائية
 */
static uint8_t Calculate_Confidence(Hadeed_Target_Feature_t *feat) {
    float score = 50.0f; // نقطة البداية (حياد)

    // أ. تعزيز الثقة بناءً على الانسجام الترددي (SMF)
    if (feat->smf.harmony_ratio > 0.80f) score += 20.0f;
    else if (feat->smf.harmony_ratio < 0.30f) score -= 15.0f;

    // ب. تعزيز الثقة بناءً على زاوية الطور (Phase Angle)
    if (feat->dp.phase_angle > 70.0f) score += 15.0f; // مؤشر قوي للمصنوع
    else if (feat->dp.phase_angle < 40.0f) score -= 10.0f; // مؤشر للطبيعي/الحديد

    // ج. فحص ثبات الإشارة (Stability)
    if (feat->dp.stability_index > 0.90f) score += 15.0f;
    else score -= 10.0f;

    // د. التحقق من السياق البصري (Vision Context)
    if (feat->visual_class_id == TARGET_ID_HOT_ROCK && feat->smf.bin_high > 100) {
        score -= 30.0f; // الكاميرا والحساس اتفقا أنها صخرة
    }

    // التأكد من بقاء النسبة بين 0 و 100
    if (score > 100.0f) score = 100.0f;
    if (score < 0.0f) score = 0.0f;

    return (uint8_t)score;
}

/* --- 2. THE MASTER FUSION FUNCTION --- */

/**
 * @brief محرك اتخاذ القرار النهائي المدمج
 * @param feat كائن البيانات الشامل للهدف
 * @return نوع الهدف النهائي المعتمد
 */
uint8_t Hadeed_Fuse_Sensors_Logic(Hadeed_Target_Feature_t *feat) {
    
    // 1. فحص "عتبة الاستيقاظ" (Wake-up Threshold)
    if (feat->peak_amplitude < SIG_NOISE_FLOOR_DEFAULT) {
        return TARGET_ID_NONE;
    }

    // 2. تحليل الفراغات والرانات (Cavity Detection)
    // المنطق: شذوذ مغناطيسي حاد + غياب إشارة النبض (PI) = تجويف صخري
    if (feat->mag_anomaly_delta > MAG_ANOMALY_THRESHOLD_NT && feat->peak_amplitude < 100.0f) {
        feat->confidence_score = 95;
        return TARGET_ID_CAVITY;
    }

    // 3. تحليل الذهب والكنوز المصنوعة (Treasure Gold)
    // المنطق: انسجام ترددي عالٍ + زاوية طور عريضة + مغناطيسية هادئة (لا يوجد حديد)
    if (feat->smf.harmony_ratio > 0.65f && feat->dp.phase_angle > 65.0f) {
        if (feat->mag_anomaly_delta < 400.0f) { // الذهب لا يشوه الحقل المغناطيسي
            feat->confidence_score = Calculate_Confidence(feat);
            return TARGET_ID_TREASURE_GOLD;
        } else {
            // إذا كان هناك انسجام ولكن مع مغناطيسية عالية = حديد قديم مصدأ (هالة)
            feat->confidence_score = 60;
            return TARGET_ID_RUSTY_IRON;
        }
    }

    // 4. تحليل الذهب الخام والشذرات الطبيعية (Natural Gold)
    // المنطق: استجابة ترددية عالية (Bin High) + انحدار خشن + سياق صخور بركانية
    if (feat->smf.bin_high > (feat->smf.bin_low * 2.0f) && feat->dp.log_slope > 0.40f) {
        // إذا أكدت الكاميرا وجود صخور بازلت، نرفع الثقة أنه ذهب طبيعي
        if (feat->visual_class_id == TARGET_ID_HOT_ROCK) {
            feat->confidence_score = 80;
            return TARGET_ID_NATURAL_GOLD;
        }
    }

    // 5. فلترة الصخور البازلتية والمعدنية المزعجة (Hot Rock Rejection)
    // المنطق: انسجام ضعيف جداً + الكاميرا ترى بازلت + الإشارة تظهر في أول التلاشي فقط
    if (feat->smf.harmony_ratio < 0.20f && feat->dp.phase_angle < 35.0f) {
        feat->confidence_score = 90;
        return TARGET_ID_HOT_ROCK;
    }

    // 6. المعادن الحديدية (Ferrous Rejection)
    if (feat->mag_anomaly_delta > MAG_ANOMALY_THRESHOLD_NT && feat->dp.log_slope < 0.25f) {
        feat->confidence_score = 85;
        return TARGET_ID_FERROUS;
    }

    // إذا لم تنطبق المعايير السابقة بوضوح
    feat->confidence_score = 30;
    return TARGET_ID_UNKNOWN;
}

/* --- 3. SPACE-TIME RECONCILIATION --- */

/**
 * @brief التوفيق بين زمن النبضة ومكان الكويل (Sync Check)
 * يمنع إصدار قرار إذا كان الجهاز يتحرك بسرعة جنونية تشتت البيانات
 */
bool Hadeed_Is_Decision_Valid(Hadeed_Target_Feature_t *feat) {
    // إذا كانت سرعة الأرجحة أكبر من 1.8 متر/ثانية، البيانات غير موثوقة للتحليل العميق
    if (feat->ground_speed > 1.8f) {
        return false;
    }
    
    // التحقق من استقرار الميلان (Tilt)
    if (feat->tilt_pitch > 45.0f || feat->tilt_roll > 45.0f) {
        return false;
    }

    return true;
}
