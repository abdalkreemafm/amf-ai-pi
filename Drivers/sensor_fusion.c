#include <math.h>

/**
 * @brief Sensor Fusion Logic for Hadeed
 * Integrates RM3100 (Magnetometer) and MPU9250 (IMU) data.
 */

typedef struct {
    float x, y, z;
} vector3;

/**
 * @brief Tilt Compensation Algorithm
 * Corrects magnetometer readings based on accelerometer pitch and roll.
 */
vector3 compensate_tilt(vector3 mag, float pitch, float roll) {
    vector3 compensated;
    
    float cos_pitch = cos(pitch);
    float sin_pitch = sin(pitch);
    float cos_roll = cos(roll);
    float sin_roll = sin(roll);

    // Standard tilt compensation formulas
    compensated.x = mag.x * cos_pitch + mag.z * sin_pitch;
    compensated.y = mag.x * sin_roll * sin_pitch + mag.y * cos_roll - mag.z * sin_roll * cos_pitch;
    compensated.z = -mag.x * cos_roll * sin_pitch + mag.y * sin_roll + mag.z * cos_roll * cos_pitch;

    return compensated;
}

/**
 * @brief PI and Magnetometer Correlation (The "Consciousness" Logic)
 * Decision: If PI detects a target but Magnetometer doesn\'t move -> Likely Non-Ferrous (Gold).
 */
int evaluate_target_consciousness(float pi_signal, float mag_delta) {
    if (pi_signal > 0.8f && mag_delta < 0.1f) {
        return 1; // High probability of Gold/Non-Ferrous
    }
    return 0; // Likely Iron or Trash
}
