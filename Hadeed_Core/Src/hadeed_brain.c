#include "hadeed_math.h"

// رموز الأوامر الصوتية للتطبيق (Voice Codes)
#define CMD_GOLD_PROBABLE  0xA1
#define CMD_IRON_DISCARD   0xA2
#define CMD_CAVITY_FOUND   0xA3
#define CMD_SLOW_DOWN      0xB1

void Hadeed_Analyze_Target(Hadeed_Signal_Feature *feat) {
    // 1. فحص المعادن الثمينة (الذهب/النحاس)
    // الذهب يتميز بانحدار لوغاريتمي حاد (Slope > 0.5) ومغناطيسية منخفضة
    if (feat->decay_slope > 0.52f && feat->mag_vector < 500.0f) {
        feat->confidence = 94.0f;
        Hadeed_Send_to_App(CMD_GOLD_PROBABLE); // أمر: "هدف ثمين بنسبة عالية"
    }
    
    // 2. فحص الفراغات والرانات
    // الفراغ يتميز بشذوذ مغناطيسي حاد دون وجود إشارة نبض حثي قوية
    else if (feat->mag_vector > 2000.0f && feat->peak_amplitude < 100.0f) {
        Hadeed_Send_to_App(CMD_CAVITY_FOUND); // أمر: "رصد فجوة صخرية أو ران"
    }
    
    // 3. عزل الحديد الصدئ
    else if (feat->mag_vector > 1500.0f && feat->decay_slope < 0.3f) {
        // تجاهل الهدف صمتاً (Mute)
    }
}
