/* المضمنات (Includes) - ربط كافة الملفات التي صممناها */
#include "main.h"
#include "cmsis_os.h"
#include "hadeed_math.h"
#include "hadeed_dsp.h"
#include "hadeed_fusion_engine.h"
#include "hadeed_fault_tolerance.h"
#include "ads1256.h"
#include "rm3100.h"

/* تعريف مقابض المهام (Task Handles) */
osThreadId_t PulseTaskHandle;
osThreadId_t BrainTaskHandle;
osThreadId_t MonitorTaskHandle;

/* المتغيرات العالمية للجهاز */
AdaptiveKalman myKalman;
Hadeed_Signal_Feature current_signal;

/* البداية الفعلية للبرنامج */
int main(void) {
    // 1. تهيئة النظام والذاكرة
    HAL_Init();
    SystemClock_Config(); // ضبط تردد المعالج لـ 480MHz (سرعة خارقة)
    
    // 2. تهيئة المنافذ (SPI للـ ADC، I2C للمغناطيسية، UART للبلوتوث)
    MX_GPIO_Init();
    MX_SPI1_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();

    // 3. تهيئة "العقل المدبر"
    Hadeed_DSP_Init(&myKalman);
    Hadeed_Health_Init(); // تشغيل نظام الحماية

    // 4. بناء نظام المهام (RTOS Scheduler)
    osKernelInitialize();

    // مهمة النبض (الأولوية القصوى - لا تحتمل التأخير)
    const osThreadAttr_t pulse_attr = {.name = "PulseTask", .priority = osPriorityRealtime};
    PulseTaskHandle = osThreadNew(Task_Pulse_and_Sample, NULL, &pulse_attr);

    // مهمة الذكاء الاصطناعي (أولوية عالية)
    const osThreadAttr_t brain_attr = {.name = "BrainTask", .priority = osPriorityHigh};
    BrainTaskHandle = osThreadNew(Task_AI_Brain, NULL, &brain_attr);

    // مهمة المراقبة والتعافي (أولوية عادية)
    const osThreadAttr_t monitor_attr = {.name = "MonitorTask", .priority = osPriorityNormal};
    MonitorTaskHandle = osThreadNew(Task_System_Monitor, NULL, &monitor_attr);

    // 5. انطلاق الوعي بالجهاز!
    osKernelStart();

    while (1) {
        // لن يصل المعالج هنا أبداً لأن المهام تديره بالأعلى
    }
}

/* ==========================================================
 * المهمة 1: إدارة النبضة والقراءة (The Heartbeat)
 * ========================================================== */
void Task_Pulse_and_Sample(void *argument) {
    for(;;) {
        // أ. إرسال النبضة (TX)
        HAL_GPIO_WritePin(TX_GATE_GPIO_Port, TX_GATE_Pin, GPIO_PIN_SET);
        osDelay(1); // نبضة لمدة 1000 ميكروثانية (قابلة للتعديل)
        HAL_GPIO_WritePin(TX_GATE_GPIO_Port, TX_GATE_Pin, GPIO_PIN_RESET);

        // ب. انتظار التلاشي (Dead-time) ثم القراءة
        // نقرأ ADS1256 عبر الـ DMA لضمان الصفر تأخير
        uint32_t raw_val = ADS1256_Read_Raw();
        
        // ج. تنقية الإشارة فوراً بكالمان التكيفي
        float clean_val = Hadeed_Adaptive_Kalman(&myKalman, (float)raw_val);
        
        // د. حفظ البيانات وتنبيه مهمة الذكاء الاصطناعي
        current_signal.peak_amplitude = clean_val;
        osThreadFlagsSet(BrainTaskHandle, 0x01); // استيقظ يا عقل لتحلل!

        osDelay(10); // تردد البحث 100 هرتز
    }
}

/* ==========================================================
 * المهمة 2: محرك اتخاذ القرار (The Brain)
 * ========================================================== */
void Task_AI_Brain(void *argument) {
    TargetType_t last_decision;
    for(;;) {
        // انتظر إشارة من مهمة النبض
        osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);

        // 1. جمع بيانات المستشعرات الأخرى (المغناطيسية والميلان)
        float mag_val = RM3100_Get_Total_Field();
        int k210_soil = K210_Get_Class();

        // 2. دمج البيانات واتخاذ القرار النهائي
        last_decision = Hadeed_Fuse_Sensors(current_signal.peak_amplitude, mag_val, k210_soil);

        // 3. إبلاغ المستخدم (صوت + شاشة)
        Hadeed_Notify_User(last_decision, 95.0f); // 95% ثقة افتراضية

        // 4. حفظ للتعلم المستقبلي
        Hadeed_Log_Trial(current_signal.peak_amplitude, mag_val, k210_soil, last_decision);
    }
}

/* ==========================================================
 * المهمة 3: المراقبة والحماية (The Sentinel)
 * ========================================================== */
void Task_System_Monitor(void *argument) {
    for(;;) {
        // فحص صحة الحساسات والبطارية
        if (!Hadeed_Check_Sensor_Status()) {
            Hadeed_Attempt_Recovery(); // إصلاح صامت
        }
        
        // فحص "ناقل الحركة التكيفي" (تغيير القوة حسب التربة)
        Hadeed_Adaptive_Gain_Control(myKalman.r);

        osDelay(500); // فحص كل نصف ثانية لتوفير الطاقة
    }
}
