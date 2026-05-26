/* hadeed_fusion_engine.c */
#include "hadeed_math.h"

// تعريف أنواع الأهداف
typedef enum {
    TARGET_NONE,
    TARGET_GOLD_PRECIOUS,
    TARGET_IRON_FERROUS,
    TARGET_CAVITY_VOID,
    TARGET_HOT_ROCK
} TargetType_t;

// دالة اتخاذ القرار النهائي
TargetType_t Hadeed_Fuse_Sensors(float pi_signal, float mag_anomaly, int k210_class) {
    
    // 1. إذا كانت الكاميرا K210 رصدت "صخرة بازلتية" (Hot Rock)
    if (k210_class == CLASS_BASALT) {
        if (pi_signal < 500.0f) return TARGET_HOT_ROCK; // تجاهل الإشارة الضعيفة
    }

    // 2. تحليل (النبض + المغناطيسية) للتمييز بين الذهب والحديد
    if (pi_signal > THRESHOLD_HIGH) {
        // إذا كانت المغناطيسية منخفضة جداً (لا يوجد حديد)
        if (mag_anomaly < 150.0f) {
            return TARGET_GOLD_PRECIOUS;
        } 
        // إذا كانت المغناطيسية عالية (تشوه في الحقل)
        else {
            return TARGET_IRON_FERROUS;
        }
    }

    // 3. تحليل الفراغات (الرانات)
    // غياب إشارة النبض مع وجود شذوذ مغناطيسي حاد (انقطاع في الصخر)
    if (pi_signal < 50.0f && mag_anomaly > 1000.0f) {
        return TARGET_CAVITY_VOID;
    }

    return TARGET_NONE;
}
