#include "model_inference.h"

/**
 * @brief Hadeed AI Model Weights (Placeholder)
 * In a production environment, this would be the output of xxd -i model.tflite
 */

const uint8_t hadeed_model_data[] = {
    0x08, 0x00, 0x00, 0x00, 0x54, 0x46, 0x4c, 0x33, // TFL3 header
    // ... model weights would go here ...
    0x00, 0x01, 0x02, 0x03
};

const int hadeed_model_data_len = sizeof(hadeed_model_data);
