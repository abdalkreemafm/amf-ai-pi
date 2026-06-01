/**
  ******************************************************************************
  * @file           : hadeed_smf.c
  * @brief          : Simultaneous Multi-Frequency (SMF) Engine via FFT
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * Uses CMSIS-DSP arm_rfft_fast_f32 for real-time spectral decomposition.
  * This is the "Secret Sauce" for rejecting hot rocks and identifying coins.
  ******************************************************************************
  */

#include "hadeed_math.h"
#include "hadeed_config.h"
#include "arm_math.h" // مكتبة المسرعات الرياضية لـ STM32

/* --- 1. FFT PRIVATE VARIABLES --- */
static arm_rfft_fast_instance_f32 fft_handler;
static float32_t fft_input_buffer[HADEED_SAMPLES_PER_PULSE];
static float32_t fft_output_buffer[HADEED_SAMPLES_PER_PULSE];
static float32_t magnitude_buffer[HADEED_SAMPLES_PER_PULSE / 2];

/**
 * @brief تهيئة محرك FFT المسرع هاردويرياً
 */
void Hadeed_SMF_Init(void) {
    // تهيئة محول فورير السريع لـ 512 نقطة
    arm_rfft_fast_init_f32(&fft_handler, HADEED_SAMPLES_PER_PULSE);
}

/**
 * @brief تحليل الطيف الترددي للإشارة المرتدة (Spectral Analysis)
 * @param target مؤشر لهيكل بيانات الهدف لتخزين النتائج
 * @param raw_buffer العينات الخام من الـ ADC بـ 24 بت
 */
void Hadeed_SMF_Process(Hadeed_Target_Feature_t *target, uint32_t *raw_buffer) {
    
    // 1. تحضير البيانات وتحويلها لـ Floating Point مع إزالة الـ DC Offset
    float32_t mean = 0;
    for (int i = 0; i < HADEED_SAMPLES_PER_PULSE; i++) {
        fft_input_buffer[i] = (float32_t)raw_buffer[i];
        mean += fft_input_buffer[i];
    }
    mean /= HADEED_SAMPLES_PER_PULSE;
    
    for (int i = 0; i < HADEED_SAMPLES_PER_PULSE; i++) {
        fft_input_buffer[i] -= mean; // تنظيف الإشارة من أي جهد ثابت
    }

    // 2. تنفيذ تحويل فورير السريع (FFT) بسرعة البرق
    arm_rfft_fast_f32(&fft_handler, fft_input_buffer, fft_output_buffer, 0);

    // 3. حساب القوة (Magnitude) لكل تردد
    arm_cmplx_mag_f32(fft_output_buffer, magnitude_buffer, HADEED_SAMPLES_PER_PULSE / 2);

    // 4. استخراج طاقة الترددات (Bucketing) بناءً على نوافذ SMF
    // الترددات تعتمد على سرعة أخذ العينات 30kSPS
    
    float32_t low_freq_power = 0;   // يمثل 4-8 kHz (للأهداف العميقة والكنوز)
    float32_t mid_freq_power = 0;   // يمثل 15-20 kHz (للذهب المتوسط)
    float32_t high_freq_power = 0;  // يمثل 30-40 kHz (للصخور والشذر الصغير)

    // تجميع الترددات المنخفضة (Bins 10 to 40)
    for (int i = 10; i < 40; i++) low_freq_power += magnitude_buffer[i];
    target->smf.bin_low = low_freq_power / 30.0f;

    // تجميع الترددات المتوسطة (Bins 60 to 100)
    for (int i = 60; i < 100; i++) mid_freq_power += magnitude_buffer[i];
    target->smf.bin_mid = mid_freq_power / 40.0f;

    // تجميع الترددات العالية (Bins 150 to 220)
    for (int i = 150; i < 220; i++) high_freq_power += magnitude_buffer[i];
    target->smf.bin_high = high_freq_power / 70.0f;

    // 5. حساب معامل الانسجام الذكي (Harmony Ratio)
    // السر الفيزيائي: المصنوع البشري (عملة) يهز كل الترددات بتناغم.
    // الصخور المشعة تهز التردد العالي فقط وتختفي في المنخفض.
    if (target->smf.bin_high > 1.0f) {
        target->smf.harmony_ratio = (target->smf.bin_low * 0.7f + target->smf.bin_mid * 0.3f) / target->smf.bin_high;
    } else {
        target->smf.harmony_ratio = 0;
    }

    // 6. التحقق من "بصمة الكنوز" (Treasure Signature)
    // إذا كان الانسجام عالياً والثبات قوياً، ارفع نسبة الثقة
    if (target->smf.harmony_ratio > 0.85f) {
        target->confidence_score += 15; // تعزيز الثقة بوجود هدف حقيقي
    } else if (target->smf.harmony_ratio < 0.20f && target->smf.bin_high > 100.0f) {
        // إذا كان التفاعل في العالي فقط، فهذه صخرة مشعة (Hot Rock)
        target->target_type = TARGET_ID_HOT_ROCK;
        target->confidence_score = 10; // خفض الثقة
    }
}
