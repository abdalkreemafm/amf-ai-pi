/* hadeed_fault_tolerance.h */
#ifndef HADEED_FAULT_TOLERANCE_H
#define HADEED_FAULT_TOLERANCE_H

#include <stdbool.h>
#include <stdint.h>

// تعريف حالات النظام
typedef enum {
    SYS_HEALTHY,
    SYS_SENSOR_FAILED,
    SYS_CRITICAL_ERROR,
    SYS_RECOVERING
} SystemState_t;

typedef struct {
    bool ads1256_ready;
    bool rm3100_ready;
    bool imu_ready;
    uint32_t last_heartbeat;
    SystemState_t current_state;
} SystemHealth_t;

// وظائف النظام
void Hadeed_Health_Init(void);
bool Hadeed_Check_Sensor_Status(void);
void Hadeed_Attempt_Recovery(void);
void Hadeed_Safe_Shutdown(void); // في حال انخفاض البطارية الحرج

#endif
