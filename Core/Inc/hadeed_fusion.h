#ifndef HADEED_FUSION_H
#define HADEED_FUSION_H

#include <stdbool.h>
#include "hadeed_math.h"
#include "hadeed_logic.h"

uint8_t Hadeed_Fuse_Sensors_Logic(Hadeed_Target_Feature_t *feat);
bool Hadeed_Is_Decision_Valid(const Hadeed_Target_Feature_t *feat);

#endif
