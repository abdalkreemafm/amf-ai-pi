/**
  ******************************************************************************
  * @file           : hadeed_coil_id.c
  * @brief          : Smart Coil Identification & Auto-Tuning Engine
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * Detects coil inductance and resistance to prevent MOSFET saturation.
  * Auto-adjusts Pulse Width and Gearbox limits based on coil size.
  ******************************************************************************
  */

#include "hadeed_math.h"
#include "hadeed_config.h"
#include "hadeed_logic.h"

/* --- 1. COIL CHARACTERISTICS DATABASE --- */
// قاعدة بيانات مرجعية لخصائص الملفات المسموح بها
static const Hadeed_Coil_Profile_t COIL_DATABASE[] = {
    {150, 350.0f, 1.2f, 100.0f, false}, // كويل 6 إنش (قناص)
    {280, 420.0f, 2.5f, 150.0f, false}, // كويل 11 إنش (قياسي)
    {450, 600.0f, 4.0f, 400.0f, false}  // كويل 18 إنش (سحيق)
};

/* --- 2. INTERNAL SENSING FUNCTIONS --- */

/**
 * @brief قياس زمن تلاشي الحقل المغناطيسي (Flyback Time)
 * يتم إرسال نبضة 10 ميكروثانية وقياس كم يستغرق الجهد للعودة للصفر
 */
static float Measure_Flyback_Time(void) {
    uint32_t start_tick, end_tick;
    
    // إرسال نبضة اختبارية قصيرة جداً وآمنة
    HAL_GPIO_WritePin(TX_GATE_GPIO_Port, TX_GATE_Pin, GPIO_PIN_SET);
    DWT_Delay_us(10); 
    HAL_GPIO_WritePin(TX_GATE_GPIO_Port, TX_GATE_Pin, GPIO_PIN_RESET);
    
    start_tick = DWT->CYCCNT; // استخدام عداد الدورات عالي الدقة
    
    // انتظار عودة الجهد من خلال الـ ADC أو مقارن (Comparator)
    // هنا نقيس الزمن حتى تهبط القراءة تحت عتبة معينة
    while(ADS1256_Read_Raw_Fast() > 500) {
        if((DWT->CYCCNT - start_tick) > 10000) break; // Timeout حماية
    }
    
    end_tick = DWT->CYCCNT;
    return (float)(end_tick - start_tick) / (SystemCoreClock / 1000000.0f); // بالمايكروثانية
}

/* --- 3. MAIN INTERFACE FUNCTIONS --- */

/**
 * @brief فحص الكويل عند التشغيل وتحديد هويته
 */
void Hadeed_AutoID_Coil(Hadeed_Coil_Profile_t *current_profile) {
    float flyback = Measure_Flyback_Time();
    
    // المنطق: الكويل الأكبر يمتلك حثاً (Inductance) أعلى وبالتالي زمن Flyback أطول
    if (flyback < 20.0f) {
        // كويل صغير (قناص)
        *current_profile = COIL_DATABASE[0];
    } 
    else if (flyback >= 20.0f && flyback < 45.0f) {
        // كويل متوسط (قياسي)
        *current_profile = COIL_DATABASE[1];
    }
    else {
        // كويل ضخم (أعماق)
        *current_profile = COIL_DATABASE[2];
    }

    // إذا كان كويل ذكي (Smart Coil) نقرأ بياناته عبر السلك الثالث (One-Wire/I2C)
    if (Hadeed_Check_Smart_Handshake()) {
        current_profile->is_smart_coil = true;
        Hadeed_Read_Smart_Coil_Data(current_profile);
    }
}

/**
 * @brief الحصول على "عرض النبضة الآمن" (Safe Pulse Width)
 * يمنع وصول الملف لحالة التشبع (Saturation) التي تحرق الترانزستورات
 */
uint16_t Hadeed_Get_Optimal_Pulse_Width(Hadeed_Coil_Profile_t *coil, uint8_t gear) {
    uint16_t base_width = coil->optimal_pulse_us;
    
    switch(gear) {
        case 1: // ECO
            return (uint16_t)(base_width * 0.5f);
        case 2: // FOCUS
            return (uint16_t)base_width;
        case 3: // HYPER
            // الحماية: لا نتجاوز أبداً ضعف العرض المثالي لتجنب الحرارة
            return (uint16_t)(base_width * 1.8f); 
        default:
            return PI_DEFAULT_PULSE_WIDTH_US;
    }
}

/**
 * @brief التحقق من وجود كويل (Coil Presence Check)
 * يمنع تشغيل البث إذا كان السلك مقطوعاً أو الكويل غير موصل
 */
bool Hadeed_Coil_Is_Present(void) {
    // قياس المقاومة الأولية صمتاً
    float r = Measure_Coil_Resistance();
    if (r < 0.2f || r > 50.0f) {
        return false; // دائرة قصر أو سلك مقطوع
    }
    return true;
}
