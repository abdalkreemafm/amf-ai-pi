/**
  ******************************************************************************
  * @file           : hadeed_safety.c
  * @brief          : System Health, Safety & Self-Healing Engine
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * Monitors battery, temperature, and sensor integrity.
  * Implements graceful degradation and automated recovery loops.
  ******************************************************************************
  */

#include "hadeed_math.h"
#include "hadeed_logic.h"
#include "hadeed_config.h"
#include "main.h"

/* --- 1. PRIVATE VARIABLES --- */
static uint8_t ads_retry_count = 0;
static uint8_t rm_retry_count = 0;
static uint32_t last_health_check_tick = 0;

/* --- 2. THERMAL & POWER MONITORING --- */

/**
 * @brief مراقبة حرارة المعالج والمكبرات التناظرية
 */
void Hadeed_Monitor_Thermal_Status(Hadeed_System_Health_t *health) {
    // قراءة مستشعر الحرارة الداخلي لمعالج STM32H7
    // ملحوظة: انحراف الحرارة يؤثر على دقة الـ ADC 24-bit
    float current_temp = Hadeed_Read_Internal_Temp();
    health->mcu_temperature = current_temp;

    if (current_temp > SYS_TEMP_LIMIT_CELSIUS) {
        // إذا سخن الجهاز في شمس اليمن، قلل طاقة البث فوراً لحماية MOSFET
        Hadeed_Set_Pulse_Gear(1); // الانتقال الإجباري لنمط ECO
        Hadeed_Log_Error("CRITICAL_TEMP_WARNING");
    }
}

/**
 * @brief مراقبة البطارية ومنع التلف الكيميائي للخلايا
 */
void Hadeed_Monitor_Battery(Hadeed_System_Health_t *health) {
    float vbatt = Hadeed_Read_Battery_Voltage();

    if (vbatt < BATT_CRITICAL_SHUTDOWN) {
        // حماية نهائية: إطفاء الجهاز قبل انهيار خلايا الليثيوم
        Hadeed_Notify_User(VOICE_CMD_BATT_LOW);
        HAL_Delay(2000);
        Hadeed_Safe_Shutdown();
    }
    else if (vbatt < BATT_LOW_THRESHOLD_VOLTS) {
        // تنبيه المنقّب وتفعيل وضع توفير الطاقة
        health->current_state = STATE_POWER_ADVISOR;
    }
}

/* --- 3. SENSOR SELF-HEALING (التعافي الذاتي) --- */

/**
 * @brief محاولة إعادة إحياء الحساسات المتوقفة صمتاً
 */
void Hadeed_Attempt_Recovery(void) {
    extern Hadeed_System_Health_t g_sys_health;
    
    // 1. فحص ADS1256 (SPI)
    if (!g_sys_health.sensor_ads1256_ok) {
        ads_retry_count++;
        Hadeed_Log_Error("RECOVERING_ADC");
        HAL_GPIO_WritePin(ADC_RST_GPIO_Port, ADC_RST_Pin, GPIO_PIN_RESET);
        HAL_Delay(10);
        HAL_GPIO_WritePin(ADC_RST_GPIO_Port, ADC_RST_Pin, GPIO_PIN_SET);
        ADS1256_Init(); // إعادة تهيئة كاملة
    }

    // 2. فحص RM3100 (I2C)
    if (!g_sys_health.sensor_rm3100_ok) {
        rm_retry_count++;
        Hadeed_Log_Error("RECOVERING_MAG");
        // إعادة تهيئة منفذ I2C بالكامل
        HAL_I2C_DeInit(&hi2c1);
        HAL_Delay(5);
        HAL_I2C_Init(&hi2c1);
        RM3100_Init();
    }

    // إذا تجاوزت المحاولات 5 مرات، أعلن فشل القطعة واستمر في العمل بما تبقى
    if (ads_retry_count > 5) {
        g_sys_health.current_state = STATE_EMERGENCY_RECOVERY;
        // هنا يمكن للجهاز العمل كـ (Magnetometer Only) إذا فشل الـ PI
    }
}

/* --- 4. MAIN HEALTH INTERFACE --- */

/**
 * @brief الفحص الدوري لصحة النظام (يستدعى من MonitorTask)
 */
bool Hadeed_Check_System_Integrity(Hadeed_System_Health_t *health) {
    bool all_ok = true;

    // فحص اتصال الحساسات عبر استجابة الـ Registers
    if (ADS1256_Ping() != HAL_OK) {
        health->sensor_ads1256_ok = false;
        all_ok = false;
    } else {
        health->sensor_ads1256_ok = true;
        ads_retry_count = 0;
    }

    if (RM3100_Ping() != HAL_OK) {
        health->sensor_rm3100_ok = false;
        all_ok = false;
    } else {
        health->sensor_rm3100_ok = true;
        rm_retry_count = 0;
    }

    // تحديث عداد الوقت (Uptime)
    health->uptime_seconds = HAL_GetTick() / 1000;

    return all_ok;
}

/**
 * @brief الإغلاق الآمن للجهاز (حماية الهاردوير)
 */
void Hadeed_Safe_Shutdown(void) {
    // 1. إيقاف نبضات البث فوراً لمنع أي شرارة
    HAL_GPIO_WritePin(TX_GATE_GPIO_Port, TX_GATE_Pin, GPIO_PIN_RESET);
    
    // 2. حفظ التعلم الميداني الأخير في الـ Flash (كما اتفقنا لحماية الذاكرة)
    Hadeed_Save_Context_To_Flash();
    
    // 3. قطع الطاقة عن الحساسات
    HAL_GPIO_WritePin(SENSOR_PWR_EN_GPIO_Port, SENSOR_PWR_EN_Pin, GPIO_PIN_RESET);
    
    // 4. الدخول في وضع النوم العميق أو إطفاء الـ MOSFET الرئيسي
    HAL_PWR_EnterSTANDBYMode();
}

/**
 * @brief مراقبة ضجيج حركة الكيبل (Cable Noise Rejection)
 */
bool Hadeed_Is_Cable_Noise(float pi_signal, float imu_accel) {
    // إذا كانت هناك إشارة قوية متزامنة بالضبط مع "هزة" عنيفة في الـ IMU
    // فهذا غالباً ضجيج فيزيائي في السلك وليس معدناً في الأرض
    if (imu_accel > 2.5f && pi_signal > 1000.0f) {
        return true;
    }
    return false;
}
