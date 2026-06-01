/**
  ******************************************************************************
  * @file           : rm3100.c
  * @brief          : Professional Driver for PNI RM3100 (Geomagnetic Sensor)
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * Optimized for high-precision Gradiometry. 
  * Uses I2C interface to communicate with the MagI2C ASIC.
  ******************************************************************************
  */

#include "rm3100.h"
#include "hadeed_math.h"
#include "hadeed_config.h"
#include "main.h"

/* --- 1. RM3100 REGISTER MAP --- */
#define RM3100_POLL_REG          0x00
#define RM3100_CMM_REG           0x01
#define RM3100_CCX_REG           0x04 // Cycle Count X
#define RM3100_CCY_REG           0x06
#define RM3100_CCZ_REG           0x08
#define RM3100_TMRC_REG          0x0B
#define RM3100_MEAS_REG          0x24 // النتائج تبدأ من هنا (X, Y, Z)
#define RM3100_BIST_REG          0x33 // فحص السلامة الذاتي
#define RM3100_REVID_REG         0x36 // WHO_AM_I

/* --- 2. INTERNAL CALIBRATION VARIABLES --- */
static float mag_baseline = 0;    // الصفر المغناطيسي للموقع الحالي
static float mag_sensitivity = 1.0f / 75.0f; // الحساسية عند 200 Cycle Count (µT per LSB)

/* --- 3. MAIN INTERFACE FUNCTIONS --- */

/**
 * @brief تهيئة الحساس وضبط الدقة العالية (13nT Resolution)
 */
void RM3100_Init(void) {
    uint8_t buf[2];

    // 1. ضبط Cycle Counts لجميع المحاور على 200 (المعيار الذهبي للدقة)
    // كلما زاد هذا الرقم زادت الدقة وقل الضجيج
    buf[0] = 0x00; buf[1] = 200; // MSB, LSB
    HAL_I2C_Mem_Write(&hi2c1, RM3100_I2C_ADDR, RM3100_CCX_REG, 1, buf, 2, 100);
    HAL_I2C_Mem_Write(&hi2c1, RM3100_I2C_ADDR, RM3100_CCY_REG, 1, buf, 2, 100);
    HAL_I2C_Mem_Write(&hi2c1, RM3100_I2C_ADDR, RM3100_CCZ_REG, 1, buf, 2, 100);

    // 2. ضبط معدل القراءة المستمر (TMRC) إلى 37 هرتز لتوفير الطاقة ومنع التداخل
    buf[0] = 0x95; 
    HAL_I2C_Mem_Write(&hi2c1, RM3100_I2C_ADDR, RM3100_TMRC_REG, 1, buf, 1, 100);

    // 3. تفعيل وضع القراءة عند الطلب (Polling Mode) لضمان التزامن مع النبضة
    buf[0] = 0x00;
    HAL_I2C_Mem_Write(&hi2c1, RM3100_I2C_ADDR, RM3100_CMM_REG, 1, buf, 1, 100);
}

/**
 * @brief قراءة المحاور الثلاثة وحساب المحصلة الإجمالية (Total Magnetic Field)
 */
float RM3100_Get_Total_Field(void) {
    uint8_t raw_data[9]; // 3 بايت لكل محور (X, Y, Z)
    int32_t x = 0, y = 0, z = 0;

    // أ. إرسال أمر "بدء القياس الآن" (Poll)
    uint8_t poll_cmd = 0x70; // اطلب القراءة للمحاور الثلاثة معاً
    HAL_I2C_Mem_Write(&hi2c1, RM3100_I2C_ADDR, RM3100_POLL_REG, 1, &poll_cmd, 1, 100);

    // ب. انتظار انتهاء القياس (DRDY Check)
    // ملاحظة: مع STM32H7 نستخدم تأخير ميكروي بسيط
    DWT_Delay_us(5000); 

    // ج. قراءة البيانات الخام بـ 24 بت
    if (HAL_I2C_Mem_Read(&hi2c1, RM3100_I2C_ADDR, RM3100_MEAS_REG, 1, raw_data, 9, 100) == HAL_OK) {
        
        // دمج البايتات (24-bit Signed Integer)
        x = (int32_t)((raw_data[0] << 16) | (raw_data[1] << 8) | raw_data[2]);
        if (x & 0x800000) x |= 0xFF000000;

        y = (int32_t)((raw_data[3] << 16) | (raw_data[4] << 8) | raw_data[5]);
        if (y & 0x800000) y |= 0xFF000000;

        z = (int32_t)((raw_data[6] << 16) | (raw_data[7] << 8) | raw_data[8]);
        if (z & 0x800000) z |= 0xFF000000;

        // د. حساب القوة الإجمالية للمجال (Vector Magnitude) بالـ نانو تسلا (nT)
        float total_field = sqrtf((float)x*x + (float)y*y + (float)z*z) * mag_sensitivity * 1000.0f;
        
        return total_field;
    }
    
    return 0.0f;
}

/**
 * @brief حساب الشذوذ المغناطيسي (Anomaly Detection)
 * هذا هو المحرك الذي يكتشف "الرانات" و"الفراغات"
 */
float RM3100_Calculate_Anomaly(float current_field) {
    static float rolling_avg = 0;
    
    // أول 100 قراءة تستخدم لبناء متوسط الحقل الطبيعي للأرض في هذا الموقع
    if (rolling_avg == 0) rolling_avg = current_field;
    rolling_avg = (rolling_avg * 0.99f) + (current_field * 0.01f);

    // حساب الفرق المطلق عن الصفر الأرضي
    float delta = fabsf(current_field - rolling_avg);
    
    return delta; // إذا تجاوزت القيمة عتبة الـ 1500nT (المعرفة في config)، فهذا هدف جيوفيزيائي
}

/**
 * @brief فحص صحة الحساس (Health Check)
 */
HAL_StatusTypeDef RM3100_Ping(void) {
    uint8_t rev_id = 0;
    if (HAL_I2C_Mem_Read(&hi2c1, RM3100_I2C_ADDR, RM3100_REVID_REG, 1, &rev_id, 1, 100) == HAL_OK) {
        // الرمز الافتراضي للحساس هو 0x22
        return (rev_id == 0x22) ? HAL_OK : HAL_ERROR;
    }
    return HAL_ERROR;
}

/**
 * @brief معايرة الصفر (Zeroing)
 * يتم استدعاؤها عندما يطلب المساعد الصوتي من المنقب تثبيت الجهاز
 */
void RM3100_Manual_Calibrate(void) {
    float sum = 0;
    for(int i=0; i<50; i++) {
        sum += RM3100_Get_Total_Field();
        HAL_Delay(10);
    }
    mag_baseline = sum / 50.0f;
}
