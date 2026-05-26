#ifndef HADEED_MATH_H
#define HADEED_MATH_H

#include <stdint.h>
#include <math.h>

// فلتر كالمان التكيفي
typedef struct {
    float q; // ضجيج العملية
    float r; // ضجيج القياس
    float x; // القيمة المقدرة
    float p; // خطأ التقدير
    float k; // معامل كالمان
    float variance_buffer[10]; 
    int v_idx;
} Hadeed_Kalman;

// بصمة الإشارة للذكاء الاصطناعي
typedef struct {
    float peak_amplitude;
    float decay_slope;    // سر التمييز بين الذهب والحديد
    float integral_area;  // حجم الهدف
    float mag_vector;     // قوة الحقل المغناطيسي
    float confidence;     // نسبة الثقة 0-100%
} Hadeed_Signal_Feature;

#endif
