/* adaptive_gain_control.c */
#include "ads1256.h"
#include "math.h"

#define IDEAL_SNR_LEVEL 15.0f // مستوى الإشارة إلى الضجيج المثالي

void Hadeed_Adaptive_Gain_Control(float current_noise_rms) {
    static uint8_t current_gain = ADS1256_GAIN_1;
    
    // إذا كان الضجيج عالياً جداً (تربة متمعدنة)، قلل الحساسية لتفادي الإشارات الكاذبة
    if (current_noise_rms > 50.0f && current_gain > ADS1256_GAIN_1) {
        current_gain--;
        ADS1256_SetGain(current_gain);
        AI_Log_Event("Soil Noise High: Reducing Sensitivity");
    }
    // إذا كانت الأرض هادئة جداً، ارفع الحساسية لاصطياد أهداف أعمق
    else if (current_noise_rms < 5.0f && current_gain < ADS1256_GAIN_64) {
        current_gain++;
        ADS1256_SetGain(current_gain);
        AI_Log_Event("Soil Quiet: Boosting Depth");
    }
}
