/* hadeed_ble_proto.c */
#include "hadeed_fusion_engine.h"

// رموز التواصل المتفق عليها مع تطبيق الموبايل
#define VOICE_CMD_GOLD    0x10  // "صديقي، رصدت هدفاً ثميناً، تمهل قليلاً"
#define VOICE_CMD_CAVITY  0x20  // "انتباه، يوجد فراغ أو ران تحت الصخرة"
#define VOICE_CMD_SPEED   0x30  // "على مهلك، خفف سرعة الأرجحة لكي لا نفقد مانبحث عنه"
#define VOICE_CMD_IRON    0x40  // (صمت - لا يرسل شيئاً)

void Hadeed_Notify_User(TargetType_t result, float confidence) {
    uint8_t payload;
    
    switch(result) {
        case TARGET_GOLD_PRECIOUS:
            payload = VOICE_CMD_GOLD;
            BLE_Send_Packet(payload, confidence);
            break;
            
        case TARGET_CAVITY_VOID:
            payload = VOICE_CMD_CAVITY;
            BLE_Send_Packet(payload, 100);
            break;
            
        case TARGET_NONE:
            // فحص سرعة الأرجحة من الـ IMU
            if (IMU_Get_Swing_Speed() > MAX_SAFE_SPEED) {
                BLE_Send_Packet(VOICE_CMD_SPEED, 0);
            }
            break;
    }
}
