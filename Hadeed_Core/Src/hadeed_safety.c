#include "main.h"

// نظام المراقب الذكي (Software Watchdog)
void Hadeed_System_Check() {
    // فحص حساس المغناطيسية RM3100
    if (HAL_I2C_IsDeviceReady(&hi2c1, RM3100_ADDR, 3, 10) != HAL_OK) {
        Hadeed_Log_Error("RM3100_ERROR");
        Hadeed_Reset_I2C(); // إعادة تهيئة الحواس صمتاً
    }
    
    // فحص حرارة المعالج لتعويض الانحراف
    uint32_t temp = HAL_ADC_GetValue(&hadc3); // مستشعر الحرارة الداخلي
    Hadeed_Compensate_Temp(temp);
}
