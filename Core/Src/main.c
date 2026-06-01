/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : The Master Orchestrator of Hadeed V7.0 Sentient Platform
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * This core file runs FreeRTOS with 5 prioritized parallel tasks.
  * Real-time Pulse Induction (PI) takes the highest priority.
  ******************************************************************************
  */

/* --- 1. INCLUDES --- */
#include "main.h"
#include "cmsis_os.h"
#include "hadeed_math.h"
#include "hadeed_logic.h"
#include "hadeed_config.h"
#include "hadeed_sync.h"

/* --- 2. GLOBAL HANDLES & OBJECTS --- */
// مقابض المهام (Task Handles)
osThreadId_t PulseTaskHandle;
osThreadId_t BrainTaskHandle;
osThreadId_t VisionSyncTaskHandle;
osThreadId_t CommTaskHandle;
osThreadId_t MonitorTaskHandle;

// مقابض التزامن (Mutexes & Semaphores)
osMutexId_t DataMutexHandle;

// كائنات البيانات المركزية لجهاز حديد
Hadeed_Target_Feature_t g_current_target;
Hadeed_System_Health_t  g_sys_health;
Hadeed_Kalman_t         g_pi_kalman;
Hadeed_Sync_Manager_t   g_sync_manager;

/* --- 3. PRIVATE FUNCTION PROTOTYPES --- */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_DMA_Init(void);

// مهام FreeRTOS
void StartPulseTask(void *argument);
void StartBrainTask(void *argument);
void StartVisionSyncTask(void *argument);
void StartCommTask(void *argument);
void StartMonitorTask(void *argument);

/* --- 4. MAIN ENTRY POINT --- */
int main(void) {
    /* تهيئة المعالج STM32H7 (480MHz) */
    HAL_Init();
    SystemClock_Config();

    /* تهيئة الحواس (Peripherals) */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_SPI1_Init();         // ADS1256
    MX_I2C1_Init();         // RM3100
    MX_USART1_UART_Init();  // BLE & K210

    /* تهيئة هياكل حديد الرياضية */
    Hadeed_DSP_Init(&g_pi_kalman);
    Hadeed_Sync_Init(&g_sync_manager);
    g_sys_health.current_state = STATE_BOOTING;

    /* تهيئة نظام التشغيل RTOS */
    osKernelInitialize();

    /* تعريف الميوتكس لحماية البيانات المشتركة */
    const osMutexAttr_t data_mutex_attr = { "DataMutex", osMutexRecursive, NULL, 0 };
    DataMutexHandle = osMutexNew(&data_mutex_attr);

    /* --- إنشاء المهام الخمس الكبرى (The Big 5) --- */

    // 1. مهمة النبض والقراءة (أولوية قصوى - Real-time)
    const osThreadAttr_t pulse_attr = { .name = "PulseTask", .priority = osPriorityRealtime, .stack_size = 1024 * 4 };
    PulseTaskHandle = osThreadNew(StartPulseTask, NULL, &pulse_attr);

    // 2. مهمة الذكاء الاصطناعي ودمج الحساسات (أولوية عالية)
    const osThreadAttr_t brain_attr = { .name = "BrainTask", .priority = osPriorityHigh, .stack_size = 1024 * 4 };
    BrainTaskHandle = osThreadNew(StartBrainTask, NULL, &brain_attr);

    // 3. مهمة مزامنة الرؤية مع K210 (أولوية عالية)
    const osThreadAttr_t vision_attr = { .name = "VisionTask", .priority = osPriorityHigh, .stack_size = 1024 * 2 };
    VisionSyncTaskHandle = osThreadNew(StartVisionSyncTask, NULL, &vision_attr);

    // 4. مهمة التواصل مع تطبيق الموبايل (أولوية عادية)
    const osThreadAttr_t comm_attr = { .name = "CommTask", .priority = osPriorityNormal, .stack_size = 1024 * 2 };
    CommTaskHandle = osThreadNew(StartCommTask, NULL, &comm_attr);

    // 5. مهمة المراقبة والتعافي الذاتي (أولوية عادية)
    const osThreadAttr_t monitor_attr = { .name = "MonitorTask", .priority = osPriorityNormal, .stack_size = 1024 * 2 };
    MonitorTaskHandle = osThreadNew(StartMonitorTask, NULL, &monitor_attr);

    /* انطلاق الوعي بالجهاز */
    osKernelStart();

    while (1) {
        // لن يصل المعالج إلى هنا أبداً
    }
}

/* --- 5. TASK IMPLEMENTATIONS --- */

/**
 * @brief المهمة 1: محرك النبضة العسكرية (Pulse Induction Engine)
 * تقوم بإطلاق النبضة وقراءة الـ 24-bit ADC في زمن حقيقي.
 */
void StartPulseTask(void *argument) {
    uint32_t raw_buffer[HADEED_SAMPLES_PER_PULSE];
    for(;;) {
        // أ. إطلاق نبضة البث (TX ON)
        HAL_GPIO_WritePin(TX_GATE_GPIO_Port, TX_GATE_Pin, GPIO_PIN_SET);
        DWT_Delay_us(PI_DEFAULT_PULSE_WIDTH_US);
        HAL_GPIO_WritePin(TX_GATE_GPIO_Port, TX_GATE_Pin, GPIO_PIN_RESET);

        // ب. التخميد النشط (Active Damping)
        HAL_GPIO_WritePin(DAMPING_SSR_GPIO_Port, DAMPING_SSR_Pin, GPIO_PIN_SET);
        DWT_Delay_us(PI_ACTIVE_DAMPING_TIME_US);
        HAL_GPIO_WritePin(DAMPING_SSR_GPIO_Port, DAMPING_SSR_Pin, GPIO_PIN_RESET);

        // ج. انتظار استقرار التربة وقراءة الـ ADC
        DWT_Delay_us(PI_START_SAMPLING_DELAY_US);
        ADS1256_Read_Burst_DMA(raw_buffer, HADEED_SAMPLES_PER_PULSE);

        // د. تنقية الإشارة وحفظ النتائج الأولية
        osMutexAcquire(DataMutexHandle, osWaitForever);
        g_current_target.peak_amplitude = Hadeed_Adaptive_Kalman(&g_pi_kalman, (float)raw_buffer[5]);
        // حساب التلاشي (سيتم كتابة الكود في hadeed_decay.c)
        osMutexRelease(DataMutexHandle);

        // هـ. تنبيه "العقل" لبدء التحليل العميق
        osThreadFlagsSet(BrainTaskHandle, 0x01);

        osDelay(PI_DEFAULT_PERIOD_MS); // تكرار النبضة (100Hz)
    }
}

/**
 * @brief المهمة 2: الفص الجبهي (The Sentient Brain)
 * تقوم بدمج النبضة مع المغناطيسية والرؤية واتخاذ القرار.
 */
void StartBrainTask(void *argument) {
    for(;;) {
        // انتظر إشارة من مهمة النبض
        osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);

        osMutexAcquire(DataMutexHandle, osWaitForever);
        
        // 1. استحضار البيانات المغناطيسية والرؤية
        g_current_target.mag_total_field = RM3100_Get_Total_Field();
        
        // 2. تحليل (SMF) والانسجام الترددي
        Hadeed_Analyze_SMF(&g_current_target);

        // 3. اتخاذ القرار النهائي (Fusion Decision)
        g_current_target.target_type = Hadeed_Fuse_Sensors(
            g_current_target.peak_amplitude, 
            g_current_target.mag_total_field, 
            g_current_target.visual_class_id
        );

        osMutexRelease(DataMutexHandle);

        // 4. إرسال الكود الصوتي فوراً للتطبيق
        osThreadFlagsSet(CommTaskHandle, 0x02);
    }
}

/**
 * @brief المهمة 3: جسر التزامن مع K210 (Vision Sync)
 * تضمن تطابق زمن الصورة مع زمن النبضة.
 */
void StartVisionSyncTask(void *argument) {
    Hadeed_Sync_Packet_t sync_packet;
    for(;;) {
        Hadeed_Sync_PreparePacket(&sync_packet);
        H7_K210_SendPacket(&sync_packet);
        
        // استقبال نتيجة الرؤية وتحديث الحالة
        Hadeed_K210_Response_t response;
        if (H7_K210_ReceiveResponse(&response) == HAL_OK) {
            g_current_target.visual_class_id = response.detected_class;
            Hadeed_Sync_ProcessResponse(&g_sync_manager, &response);
        }

        osDelay(HEARTBEAT_INTERVAL_MS);
    }
}

/**
 * @brief المهمة 4: التواصل العصبي (BLE Communication)
 * ترسل النتائج والأوامر الصوتية لهاتف المستخدم.
 */
void StartCommTask(void *argument) {
    for(;;) {
        // انتظر قراراً من "العقل"
        osThreadFlagsWait(0x02, osFlagsWaitAny, osWaitForever);

        // إرسال باكت البلوتوث المشفر
        BLE_Send_Target_Update(&g_current_target);

        // إذا كان الهدف ثميناً، أرسل أمر "افتح الـ 3D"
        if (g_current_target.confidence_score > CONFIDENCE_LEVEL_HIGH) {
            BLE_Send_Voice_Command(VOICE_CMD_GOLD_HIGH);
        }

        osDelay(BLE_UPDATE_RATE_MS);
    }
}

/**
 * @brief المهمة 5: جهاز المناعة (System Monitor)
 * تراقب الحرارة، البطارية، وسلامة الحساسات.
 */
void StartMonitorTask(void *argument) {
    for(;;) {
        // 1. مراقبة الانحراف الحراري
        g_sys_health.mcu_temperature = Internal_Temp_Sensor_Read();
        
        // 2. فحص سلامة الحساسات
        g_sys_health.sensor_ads1256_ok = (ADS1256_Check() == HAL_OK);
        g_sys_health.sensor_rm3100_ok  = (RM3100_Check() == HAL_OK);

        // 3. التعافي الذاتي إذا لزم الأمر
        if (!g_sys_health.sensor_ads1256_ok || !g_sys_health.sensor_rm3100_ok) {
            Hadeed_Attempt_Recovery();
        }

        // 4. مراقبة البطارية
        float vbatt = FuelGauge_GetVoltage();
        if (vbatt < BATT_LOW_THRESHOLD_VOLTS) {
            BLE_Send_Voice_Command(VOICE_CMD_BATT_LOW);
        }

        osDelay(1000); // مراقبة كل ثانية
    }
}

/* --- 6. SYSTEM ERROR HANDLER --- */
void Error_Handler(void) {
    Hadeed_Safe_Shutdown();
    __disable_irq();
    while (1) {
        // وميض أحمر سريع في حلقة الـ RGB للتنبيه
        HAL_GPIO_TogglePin(RGB_RED_GPIO_Port, RGB_RED_Pin);
        HAL_Delay(100);
    }
}
