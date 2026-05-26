#include <stdio.h>

/**
 * @brief Adaptive Kalman Filter for Hadeed Sentient Platform
 * Used to filter ADC data from ground noise and electromagnetic interference.
 */

typedef struct {
    float q; // Process noise covariance
    float r; // Measurement noise covariance
    float x; // Value (state)
    float p; // Estimation error covariance
    float k; // Kalman gain
} kalman_state;

void kalman_init(kalman_state* state, float q, float r, float p, float initial_value) {
    state->q = q;
    state->r = r;
    state->p = p;
    state->x = initial_value;
}

float kalman_update(kalman_state* state, float measurement) {
    // Prediction update
    state->p = state->p + state->q;

    // Measurement update
    state->k = state->p / (state->p + state->r);
    state->x = state->x + state->k * (measurement - state->x);
    state->p = (1 - state->k) * state->p;

    return state->x;
}

// Example usage for Adaptive logic
void kalman_adjust_noise(kalman_state* state, float noise_level) {
    // Adaptive logic: if noise is high, increase R to trust the model more than the measurement
    state->r = noise_level * 0.1f; 
}
