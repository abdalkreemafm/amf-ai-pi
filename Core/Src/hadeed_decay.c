/**
  ******************************************************************************
  * @file           : hadeed_decay.c
  * @brief          : Logarithmic Decay Analysis Engine (The Fingerprinter)
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * This engine extracts the "Conductivity DNA" of a metal.
  * man-made objects (coins) exhibit smooth linear log-decay.
  * natural minerals exhibit jittery, non-linear decay.
  ******************************************************************************
  */

#include "hadeed_math.h"
#include "hadeed_config.h"
#include "hadeed_logic.h"

/* --- 1. INTERNAL MATH CONSTANTS --- */
#define MIN_SIGNAL_FOR_DECAY   50.0f  // الحد الأدنى لتحليل المنحنى
#define SLOPE_STABILITY_SAMPLES 5     // عدد النقاط لفحص نعومة المنحنى

/* --- 2. MAIN LOG-DECAY ENGINE --- */

/**
 * @brief تحليل بصمة التلاشي لتمييز نوع المعدن (صناعي أم طبيعي)
 * @param target مؤشر لهيكل بيانات الهدف
 * @param raw_buffer العينات الـ 512 الملتقطة بعد النبضة
 */
void Hadeed_Analyze_Decay_Fingerprint(Hadeed_Target_Feature_t *target, uint32_t *raw_buffer) {
    
    // 1. حساب المساحة الكلية تحت المنحنى (Integral Area) لتقدير الحجم
    float area_sum = 0;
    for (int i = PI_START_SAMPLING_DELAY_US; i < HADEED_SAMPLES_PER_PULSE; i++) {
        area_sum += (float)raw_buffer[i];
    }
    target->dp.integral_area = area_sum;

    // 2. حساب الانحدار اللوغاريتمي (Logarithmic Slope)
    // نختار نقطة في بداية التلاشي ونقطة في نهايته
    float y1 = (float)raw_buffer[SMF_WINDOW_HIGH_START + 5]; 
    float y2 = (float)raw_buffer[SMF_WINDOW_MID_END];

    if (y1 > MIN_SIGNAL_FOR_DECAY && y2 > 1.0f) {
        // القانون الفيزيائي: ln(V) = -t/tau. ميل الخط المستقيم هو الموصلية
        target->dp.log_slope = (logf(y1) - logf(y2)) / (float)(SMF_WINDOW_MID_END - SMF_WINDOW_HIGH_START);
    } else {
        target->dp.log_slope = 0;
    }

    // 3. حساب معامل الخشونة (Roughness/Stability Index)
    // لفصل الذهب التكنيزي (ناعم) عن الطبيعي (خشن/مشتت)
    float slope_variability = 0;
    float last_slope = 0;
    int count = 0;

    // فحص تغير الميلان في 5 مناطق مختلفة من المنحنى
    for (int i = 0; i < SLOPE_STABILITY_SAMPLES; i++) {
        int idx = SMF_WINDOW_HIGH_START + (i * 20);
        float s = logf((float)raw_buffer[idx]) - logf((float)raw_buffer[idx + 10]);
        
        if (i > 0) {
            slope_variability += fabsf(s - last_slope);
        }
        last_slope = s;
        count++;
    }

    // الذهب التكنيزي (عملات) يعطي استقراراً عالياً (Stability Index يقترب من 1.0)
    // الصخور المشعة تعطي تذبذباً (Stability Index يهبط تحت 0.5)
    target->dp.stability_index = 1.0f - (slope_variability / (float)count);
    
    if (target->dp.stability_index < 0) target->dp.stability_index = 0;
}

/* --- 3. PHASE ANGLE CALCULATOR --- */

/**
 * @brief حساب زاوية الطور الكهرومغناطيسي
 * تساعد في عزل الحديد (زاوية منخفضة) عن المعادن الثمينة (زاوية مرتفعة)
 */
void Hadeed_Calculate_Phase_Angle(Hadeed_Target_Feature_t *target, uint32_t *raw_buffer) {
    // نستخدم نسبة التلاشي السريع إلى التلاشي البطيء
    float fast_decay = (float)raw_buffer[10] - (float)raw_buffer[30];
    float slow_decay = (float)raw_buffer[100] - (float)raw_buffer[200];

    if (slow_decay > 0) {
        // حساب الزاوية بالدرجات
        target->dp.phase_angle = atan2f(fast_decay, slow_decay) * (180.0f / 3.14159f);
    } else {
        target->dp.phase_angle = 0;
    }
}

/* --- 4. TARGET IDENTIFICATION (DNA MATCHING) --- */

/**
 * @brief مطابقة البصمة الحالية بقاعدة البيانات الفيزيائية
 */
uint8_t Hadeed_Match_Metal_DNA(Hadeed_Target_Feature_t *target) {
    
    // أ- بصمة الذهب التكنيزي (High Stability + Linear Log Decay)
    if (target->dp.stability_index > 0.85f && target->dp.log_slope > 0.45f) {
        if (target->dp.phase_angle > 65.0f) {
            return TARGET_ID_TREASURE_GOLD;
        }
    }

    // ب- بصمة الذهب الطبيعي/الشذر (Medium Stability + High Freq Response)
    if (target->dp.stability_index < 0.70f && target->dp.log_slope > 0.50f) {
        if (target->smf.bin_high > (target->smf.bin_low * 2)) {
            return TARGET_ID_NATURAL_GOLD;
        }
    }

    // ج- بصمة الحديد المصدأ (Halo Effect - Low Phase Angle)
    if (target->dp.phase_angle < 30.0f && target->mag_anomaly_delta > 1000) {
        return TARGET_ID_RUSTY_IRON;
    }

    return TARGET_ID_UNKNOWN;
}
