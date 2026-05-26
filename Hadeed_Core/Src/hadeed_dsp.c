#include "hadeed_math.h"

// تحديث فلتر كالمان ليصبح تكيفياً مع ضجيج التربة
float Hadeed_Update_Kalman(Hadeed_Kalman *kf, float measurement) {
    // حساب الانحراف المعياري لتعديل قوة الفلتر تلقائياً
    kf->variance_buffer[kf->v_idx] = measurement;
    kf->v_idx = (kf->v_idx + 1) % 10;
    
    float sum = 0, mean = 0, variance = 0;
    for(int i=0; i<10; i++) sum += kf->variance_buffer[i];
    mean = sum / 10.0f;
    for(int i=0; i<10; i++) variance += powf(kf->variance_buffer[i] - mean, 2);
    
    kf->r = 0.1f + (sqrtf(variance / 10.0f) * 2.0f); // تعديل R ديناميكياً

    // خوارزمية كالمان
    kf->p = kf->p + kf->q;
    kf->k = kf->p / (kf->p + kf->r);
    kf->x = kf->x + kf->k * (measurement - kf->x);
    kf->p = (1.0f - kf->k) * kf->p;
    
    return kf->x;
}

// حساب انحدار التلاشي اللوغاريتمي (أهم كود للتمييز)
float Hadeed_Calculate_Decay_Slope(uint32_t *buffer, uint16_t size) {
    // نأخذ عينة من بداية التلاشي ومنتصفه
    float start = (float)buffer[2];   // نتجاوز أول عينتين للضجيج
    float middle = (float)buffer[size/4];
    
    if (start <= 0 || middle <= 0) return 0;
    
    // القانون اللوغاريتمي لفصل المعادن
    return (logf(start) - logf(middle)) / (size / 4.0f);
}
