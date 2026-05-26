#ifndef MODEL_INFERENCE_H
#define MODEL_INFERENCE_H

#include <stdint.h>

/**
 * @brief Hadeed AI Inference Engine
 * This header defines the interface for the TinyML model.
 */

// Placeholder for the converted TFLite model weights
extern const uint8_t hadeed_model_data[];
extern const int hadeed_model_data_len;

typedef struct {
    float gold_probability;
    float iron_probability;
    float silver_probability;
    int detected_class;
} inference_result;

/**
 * @brief Run inference on pre-processed DSP data
 * @param dsp_features: Array of features extracted from FFT and Curve Integration
 */
inference_result run_hadeed_inference(float* dsp_features);

#endif
