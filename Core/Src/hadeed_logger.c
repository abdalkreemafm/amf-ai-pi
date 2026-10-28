#include "sd_card.h"
#include <stdio.h>
#include "hadeed_math.h"

void Hadeed_Log_Trial(float pi, float mag, int class_id, int result) {
    char log_buffer[128];
    int written = snprintf(log_buffer, sizeof(log_buffer), "%lu, %.2f, %.2f, %d, %d\n",
                           (unsigned long)HAL_GetTick(), pi, mag, class_id, result);
    if (written < 0 || (size_t)written >= sizeof(log_buffer)) return;
    SD_Write_File("LOG_DATA.CSV", log_buffer);
}
