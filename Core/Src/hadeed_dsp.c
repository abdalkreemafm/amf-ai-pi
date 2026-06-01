/**
  ******************************************************************************
  * @file           : hadeed_dsp.c
  * @brief          : Digital Signal Processing Engine (The Subconscious)
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * Implements Adaptive Kalman Filtering, Log-Decay Analysis, and Thermal 
  * Drift Compensation for 24-bit ADC data.
  ******************************************************************************
  */

#include "hadeed_dsp.h"
#include "hadeed_math.h"
#include "hadeed_config.h"

/* --- 1. INITIALIZATION --- */

/**
 * @brief تهيئة محرك المعالجة وقيم كالمان الأولية
 */
void Hadeed_DSP_Init(Hadeed_Kalman_t *kf) {
    kf->q = 0.015f;           // ضجيج العملية (سرعة استجابة الجهاز)
    kf->r = SIG_NOISE_FLOOR_DEFAULT; // ضجيج القياس الأولي
    kf->p = 1.0f;
    kf->x = 0.0f;
    kf->buffer_idx = 0;
    kf->current_std_dev = 0;

    // تصفير مخزن الضجيج
    for (int i = 0; i < HADEED_KALMAN_HISTORY; i++) {
        kf->noise_floor_buffer[i] = 0.0f;
    }
}

/* --- 2. ADAPTIVE KALMAN FILTERING --- */

/**
 * @brief فلتر كالمان التكيفي (يشعر بالتربة ويعدل نفسه)
 * @param measurement القراءة الخام من الـ ADC
 * @return القراءة المنقاة
 */
float Hadeed_Adaptive_Kalman(Hadeed_Kalman_t *kf, float measurement) {
    // أ. تحديث مخزن الضجيج لحساب الانحراف المعياري (Standard Deviation)
    kf->noise_floor_buffer[kf->buffer_idx] = measurement;
    kf->buffer_idx = (kf->buffer_idx + 1) % HADEED_KALMAN_HISTORY;

    // ب. حساب متوسط الضجيج والانحراف (لمعرفة مدى "اضطراب" التربة)
    float sum = 0, mean = 0, variance = 0;
    for (int i = 0; i < HADEED_KALMAN_HISTORY; i++) sum += kf->noise_floor_buffer[i];
    mean = sum / HADEED_KALMAN_HISTORY;

    for (int i = 0; i < HADEED_KALMAN_HISTORY; i++) {
        variance += powf(kf->noise_floor_buffer[i] - mean, 2);
    }
    kf->current_std_dev = sqrtf(variance / HADEED_KALMAN_HISTORY);

    // ج. تعديل معامل R ديناميكياً: 
    // إذا كانت الأرض مضطربة (STD_DEV عالٍ)، نرفع R لزيادة النعومة.
    // إذا كانت الأرض هادئة، نخفض R لزيادة السرعة والعمق.
    kf->r = SIG_NOISE_FLOOR_DEFAULT + (kf->current_std_dev * 2.5f);

    // د. خوارزمية التنبؤ والتصحيح (Kalman Core)
    kf->p = kf->p + kf->q;                            // التنبؤ بالخطأ
    kf->k = kf->p / (kf->p + kf->r);                  // حساب معامل الكسب
    kf->x = kf->x + kf->k * (measurement - kf->x);    // تصحيح القيمة المقدرة
    kf->p = (1.0f - kf->k) * kf->p;                   // تحديث الخطأ

    return kf->x;
}

/* --- 3. THERMAL DRIFT COMPENSATION --- */

/**
 * @brief تعويض الانحراف الحراري للمكبرات التناظرية (AD8429)
 * @param raw_signal الإشارة المنقاة
 * @param mcu_temp حرارة المعالج الحالية
 * @return الإشارة المصححة حرارياً
 */
float Hadeed_Compensate_Thermal_Drift(float raw_signal, float mcu_temp) {
    const float baseline_temp = 25.0f; // الحرارة المرجعية في المختبر
    float temp_delta = mcu_temp - baseline_temp;

    // طرح الانحراف الناتج عن تمدد المقاومات في شمس اليمن
    return raw_signal - (temp_delta * SYS_TEMP_COMPENSATION_COEFF);
}

/* --- 4. MULTI-FREQUENCY (SMF) SPECTRUM ANALYSIS --- */

/**
 * @brief تحليل استجابة الترددات المتعددة وحساب الانسجام
 */
void Hadeed_Analyze_SMF(Hadeed_Target_Feature_t *target, uint32_t *raw_buffer) {
    float low_sum = 0, mid_sum = 0, high_sum = 0;

    // 1. حساب طاقة التردد العالي (العينات المبكرة - للشذر والصخور)
    for (int i = SMF_WINDOW_HIGH_START; i < SMF_WINDOW_HIGH_END; i++) {
        high_sum += (float)raw_buffer[i];
    }
    target->smf.bin_high = high_sum / (SMF_WINDOW_HIGH_END - SMF_WINDOW_HIGH_START);

    // 2. حساب طاقة التردد المتوسط
    for (int i = SMF_WINDOW_MID_START; i < SMF_WINDOW_MID_END; i++) {
        mid_sum += (float)raw_buffer[i];
    }
    target->smf.bin_mid = mid_sum / (SMF_WINDOW_MID_END - SMF_WINDOW_MID_START);

    // 3. حساب طاقة التردد المنخفض (العينات المتأخرة - للكنوز والسبائك)
    for (int i = SMF_WINDOW_LOW_START; i < SMF_WINDOW_LOW_END; i++) {
        low_sum += (float)raw_buffer[i];
    }
    target->smf.bin_low = low_sum / (SMF_WINDOW_LOW_END - SMF_WINDOW_LOW_START);

    // 4. حساب معامل الانسجام (Harmony Ratio)
    // الذهب المصنع (عملات) يعطي انسجاماً عالياً بين الترددات
    if (target->smf.bin_high > 0) {
        target->smf.harmony_ratio = target->smf.bin_low / target->smf.bin_high;
    } else {
        target->smf.harmony_ratio = 0;
    }
}

/* --- 5. LOGARITHMIC DECAY ANALYSIS --- */

/**
 * @brief حساب الانحراف اللوغاريتمي لتمييز هوية المعدن
 */
void Hadeed_Calculate_Decay_Profile(Hadeed_Target_Feature_t *target, uint32_t *raw_buffer) {
    // نأخذ نقطتين بعيدتين في منحنى التلاشي
    float start = (float)raw_buffer[SMF_WINDOW_HIGH_START + 2]; 
    float end   = (float)raw_buffer[SMF_WINDOW_MID_END];

    if (start > 1.0f && end > 1.0f) {
        // القانون الفيزيائي: ميلان خط اللوغاريتم ثابت لكل معدن
        target->dp.log_slope = (logf(start) - logf(end)) / (float)(SMF_WINDOW_MID_END - SMF_WINDOW_HIGH_START);
    } else {
        target->dp.log_slope = 0;
    }

    // حساب ثبات الإشارة (Stability Index)
    // كلما اقتربت القيمة من 1.0 كان الهدف كتلة صلبة (تكنيزي)
    target->dp.stability_index = 1.0f - (target->current_std_dev / (target->peak_amplitude + 1.0f));
}
