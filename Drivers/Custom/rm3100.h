#ifndef HADEED_RM3100_H
#define HADEED_RM3100_H

#include "stm32h7xx_hal.h"

void RM3100_Init(void);
float RM3100_Get_Total_Field(void);
float RM3100_Calculate_Anomaly(float current_field);
HAL_StatusTypeDef RM3100_Ping(void);
void RM3100_Manual_Calibrate(void);

#endif
