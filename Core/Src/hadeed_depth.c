/**
  ******************************************************************************
  * @file           : hadeed_depth.c
  * @brief          : Real-time Depth Estimation Engine
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * Calculates target depth based on signal attenuation and coil geometry.
  * Uses logarithmic scaling to map 24-bit ADC values to centimeters.
  ******************************************************************************
  */

#include "hadeed_math.h"
#include "hadeed_config.h"
#include "hadeed_logic.h"

/* --- 1. DEPTH CALIBRATION CONSTANTS --- */
// هذه المعاملات تم ضبطها لملف بحث قياسي 11 إنش (Double-D)
#define DEPTH_K_FACTOR           45.0f   // معامل التناسب للمسافة
#define DEPTH_EXPONENT           0.333f  // جذر تكعيبي (قانون التلاشي المغناطيسي)
#define SOIL_ATTENUATION_MIN     0.85f   // معامل الخصم للتربة الملحية
#define SOIL_ATTENUATION_MAX     1.15f   // معامل التعزيز للتربة الرملية

/* --- 2. INTERNAL HELPER FUNCTIONS --- */

/**
 * @brief الحصول على معامل تضخيم المسافة بناءً على حجم الكويل
 */
static float Get_Coil_Multiplier(uint16_t diameter_mm) {
    if (diameter_mm == 0) return 1.0f;
    
    // القاعدة: الكويل الأكبر يرى أعمق ولكن بدقة أقل في تحديد المركز
    // نسبة وتناسب مع الكويل المرجعي (280mm / 11 inch)
    return (float)diameter_mm / 280.0f;
}

/**
 * @brief تصحيح العمق بناءً على نوع الهدف (الموصلية)
 */
static float Get_Material_Correction(uint8_t target_type) {
    switch(target_type) {
        case TARGET_ID_TREASURE_GOLD: return 1.05f; // الذهب المصمت يعطي إشارة أعمق
        case TARGET_ID_SILVER:        return 1.10f; // الفضة عالية الناقلية
        case TARGET_ID_FERROUS:       return 0.90f; // الحديد يشتت الإشارة ويظهر أقرب
        default:                      return 1.00f;
    }
}

/* --- 3. MAIN DEPTH ENGINE --- */

/**
 * @brief المحرك الرئيسي لحساب العمق الحقيقي
 * @param feat كائن بيانات الهدف المكتمل
 * @param coil بروفايل الكويل المركب حالياً
 * @return المسافة المقدرة بالسنتيمتر
 */
float Hadeed_Calculate_Real_Depth(Hadeed_Target_Feature_t *feat, Hadeed_Coil_Profile_t *coil) {
    
    if (feat->peak_amplitude <= 0) return 0.0f;

    // 1. الحساب الأولي بناءً على قوة الإشارة (Signal Strength to Distance)
    // نستخدم اللوغاريتم لأن حساسية الـ 24-bit ADC واسعة جداً
    float raw_depth = DEPTH_K_FACTOR * powf((1000000.0f / feat->peak_amplitude), DEPTH_EXPONENT);

    // 2. تطبيق معامل الكويل (Coil Scaling)
    float coil_mult = Get_Coil_Multiplier(coil->coil_diameter_mm);
    raw_depth *= coil_mult;

    // 3. تصحيح التربة (Soil Compensation)
    // إذا كانت التربة متمعدنة (Standard Deviation عالٍ)، الإشارة تضعف، لذا فالهدف أبعد مما يبدو
    float soil_correction = 1.0f;
    if (feat->mag_anomaly_delta > MAG_ANOMALY_THRESHOLD_NT) {
        soil_correction = SOIL_ATTENUATION_MIN; // التربة الصعبة تمتص الإشارة
    }
    raw_depth *= soil_correction;

    // 4. تصحيح نوع المعدن
    raw_depth *= Get_Material_Correction(feat->target_type);

    // 5. تعويض الميلان (Tilt Compensation)
    // إذا كان الكويل مائلاً، فالمسافة للهدف هي "الوتر" وليس "الارتفاع العمودي"
    // نستخدم جيب التمام (Cosine) لتعديل المسافة
    float tilt_rad = (feat->tilt_pitch * 3.14159f) / 180.0f;
    float vertical_depth = raw_depth * cosf(tilt_rad);

    // 6. حدود الجهاز (Sanity Check)
    if (vertical_depth < 2.0f) vertical_depth = 2.0f;    // أقل من 2 سم يعتبر ملتصقاً
    if (vertical_depth > 500.0f) vertical_depth = 0.0f; // أكثر من 5 أمتار (خارج نطاق الحساس)

    return vertical_depth;
}

/**
 * @brief تقدير "حجم الهدف" (Target Size Estimation)
 * يساعد المنقب في معرفة هل يحفر لمسمار أم لجرة
 */
float Hadeed_Estimate_Target_Size(Hadeed_Target_Feature_t *feat) {
    // الحجم يتناسب طردياً مع (المساحة تحت المنحنى) وعكسياً مع (العمق)
    if (feat->estimated_depth_cm <= 0) return 0;
    
    float size_index = (feat->dp.integral_area * feat->estimated_depth_cm) / 10000.0f;
    return size_index; // قيمة نسبية: <1 صغير، 1-5 متوسط، >10 ضخم
}
