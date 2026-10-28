#include "ads1256.h"
#include "hadeed_config.h"
#include "main.h"

#define CMD_RDATA 0x01u
#define CMD_RDATAC 0x03u
#define CMD_SDATAC 0x0Fu
#define CMD_WREG 0x50u
#define CMD_SELFCAL 0xF0u

static volatile bool burst_complete;
static volatile HAL_StatusTypeDef burst_status = HAL_ERROR;

static void ADS1256_CS(bool selected) {
    HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin,
                      selected ? GPIO_PIN_RESET : GPIO_PIN_SET);
    for (volatile int i = 0; i < 10; ++i) {}
}

static HAL_StatusTypeDef ADS1256_WaitDRDY(void) {
    uint32_t timeout = 100000u;
    while (HAL_GPIO_ReadPin(ADC_DRDY_GPIO_Port, ADC_DRDY_Pin) == GPIO_PIN_SET && timeout-- > 0u) {}
    return timeout == 0u ? HAL_TIMEOUT : HAL_OK;
}

static HAL_StatusTypeDef ADS1256_WriteReg(uint8_t reg, uint8_t value) {
    uint8_t buf[3] = {(uint8_t)(CMD_WREG | reg), 0u, value};
    ADS1256_CS(true);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, buf, sizeof(buf), 10u);
    ADS1256_CS(false);
    return status;
}

void ADS1256_Init(void) {
    uint8_t cmd = CMD_SDATAC;
    ADS1256_CS(false);
    HAL_GPIO_WritePin(ADC_RST_GPIO_Port, ADC_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10u);
    HAL_GPIO_WritePin(ADC_RST_GPIO_Port, ADC_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(50u);

    ADS1256_CS(true);
    (void)HAL_SPI_Transmit(&hspi1, &cmd, 1u, 10u);
    ADS1256_CS(false);
    (void)ADS1256_WriteReg(REG_STATUS, 0x02u);
    (void)ADS1256_WriteReg(REG_MUX, 0x01u);
    (void)ADS1256_WriteReg(REG_ADCON, ADC_DEFAULT_GAIN);
    (void)ADS1256_WriteReg(REG_DRATE, 0xF0u);

    cmd = CMD_SELFCAL;
    ADS1256_CS(true);
    (void)HAL_SPI_Transmit(&hspi1, &cmd, 1u, 10u);
    ADS1256_CS(false);
    HAL_Delay(100u);
    burst_complete = true;
    burst_status = HAL_OK;
}

int32_t ADS1256_Unpack_24BE(const uint8_t *bytes) {
    if (bytes == NULL) return 0;
    int32_t value = (int32_t)(((uint32_t)bytes[0] << 16) |
                               ((uint32_t)bytes[1] << 8) | bytes[2]);
    return (value & 0x00800000) ? (value | (int32_t)0xFF000000) : value;
}

HAL_StatusTypeDef ADS1256_Read_Raw_Status(int32_t *result) {
    if (result == NULL || ADS1256_WaitDRDY() != HAL_OK) return HAL_TIMEOUT;
    uint8_t cmd = CMD_RDATA;
    uint8_t buf[3] = {0};
    ADS1256_CS(true);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, &cmd, 1u, 10u);
    if (status == HAL_OK) {
        DWT_Delay_us(7u);
        status = HAL_SPI_Receive(&hspi1, buf, sizeof(buf), 10u);
    }
    ADS1256_CS(false);
    if (status == HAL_OK) *result = ADS1256_Unpack_24BE(buf);
    return status;
}

int32_t ADS1256_Read_Raw(void) {
    int32_t result = 0;
    (void)ADS1256_Read_Raw_Status(&result);
    return result;
}

HAL_StatusTypeDef ADS1256_Read_Burst_DMA(uint8_t *output_bytes, uint16_t num_samples) {
    if (output_bytes == NULL || num_samples == 0u || num_samples > (UINT16_MAX / 3u)) return HAL_ERROR;
    if (!burst_complete && burst_status == HAL_BUSY) return HAL_BUSY;

    uint8_t cmd = CMD_RDATAC;
    burst_complete = false;
    burst_status = HAL_BUSY;
    ADS1256_CS(true);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, &cmd, 1u, 10u);
    if (status == HAL_OK) {
        DWT_Delay_us(7u);
        status = HAL_SPI_Receive_DMA(&hspi1, output_bytes, (uint16_t)(num_samples * 3u));
    }
    if (status != HAL_OK) {
        ADS1256_CS(false);
        burst_status = status;
    }
    return status;
}

bool ADS1256_Burst_Complete(void) { return burst_complete; }
HAL_StatusTypeDef ADS1256_Burst_Status(void) { return burst_status; }

HAL_StatusTypeDef ADS1256_Abort_Burst(void) {
    HAL_StatusTypeDef status = HAL_SPI_Abort(&hspi1);
    ADS1256_CS(false);
    burst_status = status;
    burst_complete = true;
    return status;
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi == &hspi1) {
        ADS1256_CS(false);
        burst_status = HAL_OK;
        burst_complete = true;
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
    if (hspi == &hspi1) {
        ADS1256_CS(false);
        burst_status = HAL_ERROR;
        burst_complete = true;
    }
}

void ADS1256_SetGain(uint8_t gain) {
    (void)ADS1256_WriteReg(REG_ADCON, gain);
}

HAL_StatusTypeDef ADS1256_Ping(void) {
    uint8_t cmd = 0x10u;
    uint8_t id = 0u;
    ADS1256_CS(true);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, &cmd, 1u, 10u);
    if (status == HAL_OK) status = HAL_SPI_Receive(&hspi1, &id, 1u, 10u);
    ADS1256_CS(false);
    if (status != HAL_OK) return status;
    return ((id >> 4) == 0x03u) ? HAL_OK : HAL_ERROR;
}
