#ifndef HADEED_DSP_H
#define HADEED_DSP_H

#include <stdint.h>
#include "hadeed_math.h"

void Hadeed_DSP_Init(Hadeed_Kalman_t *kf);
float Hadeed_Adaptive_Kalman(Hadeed_Kalman_t *kf, float measurement);
float Hadeed_Compensate_Thermal_Drift(float raw_signal, float mcu_temp);
void Hadeed_Analyze_SMF(Hadeed_Target_Feature_t *target, const int32_t *raw_buffer, uint16_t sample_count);
void Hadeed_Calculate_Decay_Profile(Hadeed_Target_Feature_t *target, const int32_t *raw_buffer, uint16_t sample_count);

#endif
