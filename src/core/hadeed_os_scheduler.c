/* hadeed_os_scheduler.c */
#include "FreeRTOS.h"
#include "task.h"
#include "hadeed_fault_tolerance.h"

// أولويات المهام (عسكرية)
#define PRIORITY_PULSE_ADC    configMAX_PRIORITIES - 1 // أولوية قصوى
#define PRIORITY_AI_ANALYSIS  configMAX_PRIORITIES - 3
#define PRIORITY_UI_UPDATE    configMAX_PRIORITIES - 5

// 1. مهمة النبض والقراءة (Real-Time)
void Task_Pulse_and_Sample(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for(;;) {
        // بث النبضة (Pulse)
        Hadeed_Transmit_Pulse();
        
        // قراءة ADC باستخدام DMA (لا تأخير)
        ADS1256_Request_Data_DMA();
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10)); // تكرار كل 10ms (100Hz)
    }
}

// 2. مهمة الذكاء الاصطناعي (Background Analysis)
void Task_AI_Brain(void *pvParameters) {
    for(;;) {
        // انتظر إشارة من الـ ADC بأن البيانات جاهزة
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 0) {
            AI_Process_Inference(); // تنفيذ التمييز بين الذهب والحديد
        }
    }
}

// 3. مهمة المراقبة (Supervisor)
void Task_System_Monitor(void *pvParameters) {
    for(;;) {
        if (!Hadeed_Check_Sensor_Status()) {
            Hadeed_Attempt_Recovery();
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // افحص كل نصف ثانية
    }
}
