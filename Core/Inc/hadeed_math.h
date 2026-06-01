/**
  ******************************************************************************
  * @file           : hadeed_math.h
  * @brief          : The Mathematical DNA of Hadeed V7.0 Sentient Platform
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * This file defines the global structures for SMF analysis, Logarithmic Decay,
  * Sensor Fusion, and Autonomous AI weights.
  ******************************************************************************
  */

#ifndef HADEED_MATH_H
#define HADEED_MATH_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* --- CONSTANTS & CONFIGURATION --- */
#define HADEED_ADC_CHANNELS      4       // لدعم الترقية لـ V3 مستقبلاً
#define HADEED_SAMPLES_PER_PULSE 512     // عدد العينات لكل نبضة واحدة
#define HADEED_SMF_BINS          3       // ترددات: منخفض، متوسط، عالي
#define HADEED_KALMAN_HISTORY    10      // لتتبع استقرار التربة

/* --- 1. ADAPTIVE DSP STRUCTURES --- */

/**
 * @brief فلتر كالمان التكيفي (يشعر بضجيج التربة ويعدل قوة الفلترة)
 */
typedef struct {
    float q;                  // ضجيج العملية (Process Noise)
    float r;                  // ضجيج القياس (Measurement Noise) - يتغير ديناميكياً
    float x;                  // القيمة المقدرة الحالية
    float p;                  // تقدير الخطأ
    float k;                  // معامل كالمان (Gain)
    float noise_floor_buffer[HADEED_KALMAN_HISTORY];
    uint8_t buffer_idx;
    float current_std_dev;    // الانحراف المعياري للتربة الحالية
} Hadeed_Kalman_t;

/* --- 2. MULTI-FREQUENCY & DECAY STRUCTURES --- */

/**
 * @brief البصمة الطيفية للترددات المتعددة (SMF)
 */
typedef struct {
    float bin_low;            // استجابة الكتل العميقة (4-8 kHz)
    float bin_mid;            // استجابة الذهب المتوسط (15-20 kHz)
    float bin_high;           // استجابة الشذرات والصخور (40+ kHz)
    float harmony_ratio;      // معامل الانسجام (السر في كشف المصنوع البشري)
} Hadeed_SMF_Data_t;

/**
 * @brief تحليل بصمة التلاشي (Decay Fingerprint)
 */
typedef struct {
    float log_slope;          // الانحدار اللوغاريتمي (تمييز نوع المعدن)
    float stability_index;    // معامل الاستقرار (الفرق بين الصلب والمشتت)
    float integral_area;      // المساحة تحت المنحنى (حجم الهدف)
    float phase_angle;        // زاوية الطور الكهرومغناطيسي
} Hadeed_Decay_Profile_t;

/* --- 3. SENSOR FUSION & AI DATA OBJECTS --- */

/**
 * @brief الكائن الشامل للهدف (The Target Object)
 * يجمع البيانات من النبضة والمغناطيسية والرؤية والعمق
 */
typedef struct {
    float peak_amplitude;     // أقصى قوة للإشارة
    Hadeed_SMF_Data_t smf;    // بيانات الترددات
    Hadeed_Decay_Profile_t dp; // بيانات التلاشي
    
    // البيانات المغناطيسية (RM3100)
    float mag_total_field;    // محصلة الحقل المغناطيسي
    float mag_anomaly_delta;  // مقدار الشذوذ عن الصفر الأرضي
    
    // البيانات المكانية والحركية (IMU)
    float tilt_pitch;         // ميلان الكويل (لتعويض الخطأ)
    float tilt_roll;
    float ground_speed;       // سرعة الأرجحة
    
    // بيانات الرؤية (K210)
    uint8_t visual_class_id;  // تصنيف الكاميرا للتربة/الصخور
    
    // النتائج النهائية
    float estimated_depth_cm; // العمق التقديري
    uint8_t confidence_score; // نسبة الثقة (0-100%)
    uint8_t target_type;      // (راجع hadeed_logic.h للأكواد)
} Hadeed_Target_Feature_t;

/* --- 4. HARDWARE AWARENESS STRUCTURES --- */

/**
 * @brief تعريف هوية الكويل الميكانيكي (Coil Fingerprint)
 */
typedef struct {
    uint16_t coil_diameter_mm; // قطر الملف
    float inductance_uh;       // الحث (L)
    float resistance_ohm;      // المقاومة (R)
    float optimal_pulse_us;    // عرض النبضة المثالي لهذا الملف
    bool is_smart_coil;        // هل يحتوي على STM32G0 داخلي؟
} Hadeed_Coil_Profile_t;

/**
 * @brief كيمياء التربة (Wien Bridge Data)
 */
typedef struct {
    float conductivity_ms;    // ناقلية التربة (الأملاح)
    float moisture_pct;       // نسبة الرطوبة
    float phase_shift_1khz;   // إزاحة الطور الكيميائي
} Hadeed_Soil_Chemistry_t;

/* --- 5. SYSTEM HEALTH & SECURITY --- */

/**
 * @brief حالة المناعة والتعافي للنظام
 */
typedef struct {
    bool sensor_ads1256_ok;
    bool sensor_rm3100_ok;
    bool sensor_imu_ok;
    bool k210_link_ok;
    float mcu_temperature;    // لمراقبة الانحراف الحراري للمكبرات
    uint32_t uptime_seconds;
    uint8_t security_status;  // 0: مشفر، 1: غير مصدق، 2: تم الاختراق
} Hadeed_System_Health_t;

#endif /* HADEED_MATH_H */
