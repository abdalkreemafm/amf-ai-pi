#ifndef HADEED_MATH_H
#define HADEED_MATH_H

#include <stdint.h>
#include <math.h>

// هيكل فلتر كالمان التكيفي
typedef struct {
    float q, r, x, p, k;
    float variance_buffer[10]; 
    int v_idx;
} Hadeed_Kalman;

// هيكل بصمة الهدف
typedef struct {
    float peak_amplitude;
    float decay_slope;
    float mag_vector;
    float confidence;
} Hadeed_Signal_Feature;

#endif
