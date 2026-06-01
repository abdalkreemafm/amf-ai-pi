/**
  ******************************************************************************
  * @file           : ads1256.c
  * @brief          : Professional Driver for TI ADS1256 (24-bit ADC)
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * Optimized for STM32H7 @ 480MHz. Uses SPI Mode 1 (CPOL=0, CPHA=1).
  * Supports high-speed burst sampling for Pulse Induction decay curves.
  ******************************************************************************
  */

#include "ads1256.h"
#include "hadeed_config.h"
#include "main.h"

/* --- 1. ADS1256 REGISTER COMMANDS --- */
#define CMD_WAKEUP    0x00
#define CMD_RDATA     0x01
#define CMD_RDATAC    0x03
#define CMD_SDATAC    0x0F
#define CMD_RREG      0x10
#define CMD_WREG      0x50
#define CMD_SELFCAL   0xF0
#define CMD_SYNC      0xFC
#define CMD_STANDBY   0xFD

/* --- 2. INTERNAL HELPERS --- */

/**
 * @brief التحكم في خط اختيار الشريحة (CS) مع تأخير بسيط لتناسب سرعة H7
 */
static void ADS1256_CS(bool state) {
    HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, state ? GPIO_PIN_RESET : GPIO_PIN_SET);
    // تأخير ميكروي بسيط لأن المعالج أسرع بمراحل من الـ ADC
    for(volatile int i=0; i<10; i++); 
}

/**
 * @brief انتظار إشارة الجاهزية (DRDY) من الحساس
 */
static void ADS1256_WaitDRDY(void) {
    uint32_t timeout = 100000;
    while (HAL_GPIO_ReadPin(ADC_DRDY_GPIO_Port, ADC_DRDY_Pin) == GPIO_PIN_SET && timeout--) {
        // ننتظر حتى يهبط خط DRDY للصفر
    }
}

/**
 * @brief إرسال أمر وكتابة قيمة في سجل (Register)
 */
static void ADS1256_WriteReg(uint8_t reg, uint8_t value) {
    ADS1256_CS(true);
    uint8_t buf[3];
    buf[0] = CMD_WREG | reg;
    buf[1] = 0x00; // كتابة سجل واحد
    buf[2] = value;
    HAL_SPI_Transmit(&hspi1, buf, 3, 10);
    ADS1256_CS(false);
}

/* --- 3. MAIN PUBLIC FUNCTIONS --- */

/**
 * @brief تهيئة الحساس وضبط الحساسية القصوى (Gain 64) وسرعة 30kSPS
 */
void ADS1256_Init(void) {
    ADS1256_CS(false);
    HAL_GPIO_WritePin(ADC_RST_GPIO_Port, ADC_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(ADC_RST_GPIO_Port, ADC_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(50);

    ADS1256_CS(true);
    uint8_t cmd = CMD_SDATAC; // إيقاف القراءة المستمرة للبرمجة
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
    
    // 1. ضبط سجل الحالة (STATUS): تفعيل الـ Buffer لرفع المقاومة المدخلة
    ADS1256_WriteReg(REG_STATUS, 0x02); 
    
    // 2. ضبط المدخلات (MUX): قناة 0 موجبة وقناة 1 سالبة (Differential)
    ADS1256_WriteReg(REG_MUX, 0x01); 
    
    // 3. ضبط التضخيم (ADCON): Gain 64 للحساسية المفرطة
    ADS1256_WriteReg(REG_ADCON, ADC_DEFAULT_GAIN); 
    
    // 4. ضبط سرعة البيانات (DRATE): 30,000 عينة في الثانية
    ADS1256_WriteReg(REG_DRATE, 0xF0); 

    // 5. إجراء معايرة ذاتية (Self-Calibration)
    cmd = CMD_SELFCAL;
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
    HAL_Delay(100);
    
    ADS1256_CS(false);
}

/**
 * @brief قراءة عينة واحدة بـ 24 بت وتحويلها لرقم 32 بت معالج
 */
int32_t ADS1256_Read_Raw(void) {
    uint8_t buf[3] = {0};
    int32_t result = 0;

    ADS1256_WaitDRDY();
    ADS1256_CS(true);

    uint8_t cmd = CMD_RDATA;
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
    
    // تأخير زمن قراءة البيانات (t6) حسب الداتا شيت
    DWT_Delay_us(7); 

    HAL_SPI_Receive(&hspi1, buf, 3, 10);
    ADS1256_CS(false);

    // دمج الـ 3 بايت في رقم واحد 24-بت (Signed)
    result = (int32_t)((buf[0] << 16) | (buf[1] << 8) | buf[2]);

    // معالجة الإشارة (Sign Extension)
    if (result & 0x800000) {
        result |= 0xFF000000;
    }

    return result;
}

/**
 * @brief القراءة السريعة "دفعات" (Burst Read) باستخدام الـ DMA
 * هذه الدالة هي سر تصوير الـ 3D وتحليل التلاشي
 */
void ADS1256_Read_Burst_DMA(uint32_t *output_buffer, uint16_t num_samples) {
    ADS1256_CS(true);
    
    // تفعيل وضع القراءة المستمرة (RDATAC)
    uint8_t cmd = CMD_RDATAC;
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
    DWT_Delay_us(7);

    // استلام البيانات عبر الـ DMA مباشرة للذاكرة لضمان عدم استهلاك المعالج
    // يتم استلام 3 بايت لكل عينة
    HAL_SPI_Receive_DMA(&hspi1, (uint8_t*)output_buffer, num_samples * 3);
    
    // المايسترو (main.c) سينتظر انتهاء الـ DMA لمعالجة المصفوفة
}

/**
 * @brief تغيير الحساسية (Gain) ديناميكياً بناءً على أوامر الـ AI
 */
void ADS1256_SetGain(uint8_t gain) {
    ADS1256_WriteReg(REG_ADCON, gain);
    // يجب إجراء معايرة سريعة بعد تغيير الـ Gain
    uint8_t cmd = CMD_SELFCAL;
    ADS1256_CS(true);
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
    ADS1256_CS(false);
}

/**
 * @brief فحص اتصال الحساس (Health Check)
 */
HAL_StatusTypeDef ADS1256_Ping(void) {
    ADS1256_CS(true);
    uint8_t cmd = 0x10; // Read ID Register
    uint8_t id = 0;
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
    HAL_SPI_Receive(&hspi1, &id, 1, 10);
    ADS1256_CS(false);
    
    return (id >> 4 == 0x03) ? HAL_OK : HAL_ERROR;
}
