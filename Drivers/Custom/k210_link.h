#ifndef HADEED_K210_LINK_H
#define HADEED_K210_LINK_H

#include <stdbool.h>
#include "hadeed_sync.h"
#include "stm32h7xx_hal.h"

void H7_K210_Init(void);
HAL_StatusTypeDef H7_K210_SendPacket(Hadeed_Sync_Packet_t *packet);
HAL_StatusTypeDef H7_K210_ReceiveResponse(Hadeed_K210_Response_t *response);
bool H7_K210_Is_Link_Alive(void);

#endif
