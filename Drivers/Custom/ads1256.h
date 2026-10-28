#ifndef HADEED_ADS1256_H
#define HADEED_ADS1256_H

#include <stdbool.h>
#include <stdint.h>
#include "stm32h7xx_hal.h"

void ADS1256_Init(void);
HAL_StatusTypeDef ADS1256_Read_Raw_Status(int32_t *result);
int32_t ADS1256_Read_Raw(void);
HAL_StatusTypeDef ADS1256_Read_Burst_DMA(uint8_t *output_bytes, uint16_t num_samples);
bool ADS1256_Burst_Complete(void);
HAL_StatusTypeDef ADS1256_Burst_Status(void);
HAL_StatusTypeDef ADS1256_Abort_Burst(void);
int32_t ADS1256_Unpack_24BE(const uint8_t *bytes);
void ADS1256_SetGain(uint8_t gain);
HAL_StatusTypeDef ADS1256_Ping(void);

#endif
