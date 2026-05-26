/* hadeed_hadeed_logger.c */
#include "sd_card.h"

void Hadeed_Log_Trial(float pi, float mag, int class, TargetType_t result) {
    char log_buffer[128];
    
    // تنسيق البيانات بصيغة CSV ليفهمها الذكاء الاصطناعي
    sprintf(log_buffer, "%lu, %.2f, %.2f, %d, %d\n", 
            HAL_GetTick(), pi, mag, class, result);
            
    // حفظ في ملف اليوم على كرت الذاكرة
    SD_Write_File("LOG_DATA.CSV", log_buffer);
}
