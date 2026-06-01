/**
  ******************************************************************************
  * @file           : wien_bridge.c
  * @brief          : Soil Chemistry & Conductivity Analysis Engine
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * Implements 1kHz AC excitation to measure soil impedance.
  * Adjusts PI damping parameters based on chemical feedback.
  ******************************************************************************
  */

#include "wien_bridge.h"
#include "hadeed_math.h"
#include "hadeed_config.h"
#include "main.h"

/* --- 1. PRIVATE CONSTANTS --- */
#define WIEN_AC_FREQ_HZ          1000   // تردد الإثارة (1 كيلو هرتز)
#define WIEN_SAMPLES_COUNT       100    // عدد العينات لحساب الـ RMS
#define CONDUCTIVITY_K_FACTOR    1.25f  // معامل تحويل الجهد إلى ناقلية (mS)

/* --- 2. INTERNAL CALIBRATION --- */
static float bridge_offset_v = 0.0f;    // معايرة الصفر في الهواء

/* --- 3. MAIN INTERFACE FUNCTIONS --- */

/**
 * @brief تهيئة قنطرة وين وتفعيل مولد التردد (AC Generator)
 */
void Hadeed_WienBridge_Init(void) {
    // 1. تفعيل الـ PWM أو الـ DAC لتوليد موجة 1kHz
    // نستخدم Timer مخصص لهذه المهمة لضمان استمرارها في الخلفية
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); 

    // 2. إجراء معايرة أولية (Zero Cal) في الهواء
    float sum = 0;
    for(int i=0; i<50; i++) {
        sum += Hadeed_WienBridge_Read_Raw_V();
        HAL_Delay(5);
    }
    bridge_offset_v = sum / 50.0f;
}

/**
 * @brief قراءة الجهد الخام من مخرج قنطرة وين
 */
float Hadeed_WienBridge_Read_Raw_V(void) {
    uint32_t adc_val = 0;
    // قراءة القناة المخصصة لقنطرة وين (عبر المحول الداخلي أو ADS1256)
    if (HAL_ADC_Start(&hadc1) == HAL_OK) {
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
            adc_val = HAL_ADC_GetValue(&hadc1);
        }
    }
    return (float)adc_val * (3.3f / 65535.0f); // تحويل لجهد (STM32H7 16-bit ADC)
}

/**
 * @brief التحليل الشامل لكيمياء التربة
 * @param soil مؤشر لهيكل بيانات الكيمياء المراد تعبئته
 */
void Hadeed_WienBridge_Analyze(Hadeed_Soil_Chemistry_t *soil) {
    float v_sum_sq = 0;
    float v_instant = 0;
    
    // 1. حساب الجهد الفعال (RMS Voltage) لتحليل الناقلية
    for(int i=0; i < WIEN_SAMPLES_COUNT; i++) {
        v_instant = Hadeed_WienBridge_Read_Raw_V() - bridge_offset_v;
        v_sum_sq += (v_instant * v_instant);
        DWT_Delay_us(100); // أخذ عينات موزعة على طول الموجة (1kHz)
    }
    
    float v_rms = sqrtf(v_sum_sq / WIEN_SAMPLES_COUNT);

    // 2. حساب الناقلية (Conductivity) بالـ milli-Siemens
    soil->conductivity_ms = v_rms * CONDUCTIVITY_K_FACTOR;

    // 3. تقدير نسبة الرطوبة (Moisture Estimation)
    // الرطوبة العالية تزيد من سعة التربة (Capacitance) وتغير الجهد
    soil->moisture_pct = (soil->conductivity_ms / 10.0f) * 100.0f;
    if (soil->moisture_pct > 100.0f) soil->moisture_pct = 100.0f;

    // 4. حساب إزاحة الطور (Phase Shift)
    // يتم عن طريق مقارنة زمن العبور للصفر (Zero Crossing) بين البث والاستقبال
    soil->phase_shift_1khz = Hadeed_WienBridge_Measure_Phase();

    // 5. وظيفة الوعي: تعديل زمن التخميد (Damping) للنبضة الحثية
    // إذا كانت التربة ملحية جداً (ناقلية عالية)، نحتاج لتخميد أبطأ قليلاً لمنع الرنين
    Hadeed_Adjust_Damping_To_Soil(soil->conductivity_ms);
}

/**
 * @brief قياس إزاحة الطور بين إشارة المرجع وإشارة التربة
 */
float Hadeed_WienBridge_Measure_Phase(void) {
    // خوارزمية قياس الفرق الزمني بالمايكروثانية
    // تساعد في كشف "التربة الحديدية" (Mineralized Soil)
    uint32_t t_ref = 0, t_soil = 0;
    
    // انتظار إشارة المرجع (1kHz PWM) لتصل للصفر
    while(HAL_GPIO_ReadPin(WIEN_REF_GPIO_Port, WIEN_REF_Pin) == GPIO_PIN_RESET);
    t_ref = DWT->CYCCNT;
    
    // انتظار إشارة التربة لتصل لذروتها (أو عبور الصفر)
    while(Hadeed_WienBridge_Read_Raw_V() < (bridge_offset_v + 0.1f));
    t_soil = DWT->CYCCNT;

    float diff_us = (float)(t_soil - t_ref) / (SystemCoreClock / 1000000.0f);
    
    // تحويل الزمن إلى درجات (360 درجة لـ 1000 ميكروثانية)
    float phase_deg = (diff_us / 1000.0f) * 360.0f;
    
    return phase_deg;
}

/**
 * @brief ضبط الهاردوير بناءً على الكيمياء (Auto-Tune)
 */
void Hadeed_Adjust_Damping_To_Soil(float conductivity) {
    extern uint16_t g_active_damping_us;
    
    if (conductivity > 5.0f) {
        // تربة ملحية/رطبة جداً: زيادة وقت التخميد قليلاً لزيادة الاستقرار
        g_active_damping_us = PI_ACTIVE_DAMPING_TIME_US + 2;
    } else {
        // تربة جافة ونظيفة: تخميد حاد وسريع لأقصى حساسية
        g_active_damping_us = PI_ACTIVE_DAMPING_TIME_US;
    }
}
