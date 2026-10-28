#ifndef HADEED_COMM_H
#define HADEED_COMM_H

#include <stdbool.h>
#include <stdint.h>
#include "hadeed_math.h"

void Hadeed_Comm_Send_Target_Update(const Hadeed_Target_Feature_t *target);
void Hadeed_Comm_Send_Voice_Trigger(uint16_t voice_packet);
void Hadeed_Comm_Send_3D_Frame(float mag_val, float pi_val, float x, float y);
bool Hadeed_Comm_Perform_Security_Handshake(void);

#endif
