/**
  ******************************************************************************
  * @file           : hadeed_empathy.h
  * @brief          : Emotional AI & Empathy Engine Logic (The Companion's Soul)
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * This file contains the linguistic matrices for the AI Companion. 
  * The App uses these indices to play randomized, context-aware responses.
  ******************************************************************************
  */

#ifndef HADEED_EMPATHY_H
#define HADEED_EMPATHY_H

#include <stdint.h>

/* --- 1. AI PERSONALITY MODES (أطوار شخصية المساعد) --- */
typedef enum {
    EMPATHY_VERBOSE,   // الطور الثرثار: توجيه مكثف للمبتدئين (أول 10 دقائق)
    EMPATHY_BALANCED,  // الطور المتوازن: تدخل عند الضرورة فقط
    EMPATHY_ZEN,       // طور الزن: صمت شبه مطبق، تنبيهات للأهداف الكبرى فقط
    EMPATHY_OFF        // إيقاف المساعد الصوتي (نغمات فقط)
} Empathy_Level_t;

/* --- 2. DYNAMIC PHRASE MATRICES (مصفوفات الجمل الديناميكية) --- */
/* ملاحظة: هذه الجمل مخزنة في التطبيق، وهذا التعريف لضمان تزامن الـ IDs */

// فئة: التنبيه لسرعة الأرجحة (Speed/Pacing Alerts)
#define EMPATHY_MSG_SLOW_DOWN_1    "يا صديقي، تمهل قليلاً لكي لا يذهب عملنا سدى."
#define EMPATHY_MSG_SLOW_DOWN_2    "خطواتك سريعة بعض الشيء، الأهداف العميقة تحتاج مسحاً هادئاً."
#define EMPATHY_MSG_SLOW_DOWN_3    "على مهلك.. دعنا نأخذ وقتنا لنرى ما تحت هذه الصخور بوضوح."
#define EMPATHY_MSG_SLOW_DOWN_4    "هدئ السرعة قليلاً، نحن نقترب من منطقة واعدة."

// فئة: تشجيع عند رصد إشارة ثابتة (Target Found Motivation)
#define EMPATHY_MSG_GOLD_FOUND_1   "استعد! رصدتُ هدفاً ثميناً بنسبة ثقة عالية جداً."
#define EMPATHY_MSG_GOLD_FOUND_2   "إشارة ذهب رائعة وصافية! نوصي بالتحقيق والحفر بحذر."
#define EMPATHY_MSG_GOLD_FOUND_3   "هناك شيء مذهل تحت الكويل الآن، انظر للشاشة لتأكيد المركز."

// فئة: التعامل مع الصخور البازلتية والمشعة (Hot Rock Handling)
#define EMPATHY_MSG_HOTROCK_1      "لا تقلق، هذه مجرد صخرة بازلتية مخادعة، قمت بتصفيتها."
#define EMPATHY_MSG_HOTROCK_2      "الأرض متمعدنة هنا، سأقوم بتهدئة الحساسية تلقائياً لكي لا نزعجك."
#define EMPATHY_MSG_HOTROCK_3      "تجاهل هذا الصوت، إنها ضوضاء التربة الطبيعية، تابع المشي."

// فئة: نصائح الارتفاع والوضعية (Posture/Coil Height)
#define EMPATHY_MSG_COIL_HIGH_1    "أخفض ملف البحث قليلاً نحو الأرض لزيادة عمق الاختراق."
#define EMPATHY_MSG_COIL_HIGH_2    "اجعل الكويل موازياً للصخر تماماً للحصول على أفضل قراءة."

/* --- 3. EMPATHY ENGINE STRUCTURE (هيكل محرك التعاطف) --- */
typedef struct {
    Empathy_Level_t personality;   // مستوى تدخل المساعد
    uint32_t last_talk_timestamp;  // لمنع الثرثرة المتكررة في وقت قصير
    uint16_t targets_found_today;  // عداد الأهداف اليومي للتحفيز
    uint8_t current_variation_idx; // لضمان اختيار جملة مختلفة كل مرة
} Hadeed_Empathy_Engine_t;

/* --- 4. EMPATHY LOGIC FUNCTIONS (دوال المنطق العاطفي) --- */

/**
 * @brief تختار رقم الجملة العشوائية بناءً على الكود المرسل
 * @param cmd_code الكود القادم من hadeed_logic.h
 * @return variation_id (0-3) ليتم إرساله مع الباكت للبلوتوث
 */
static inline uint8_t Get_Empathy_Variation(uint8_t cmd_code) {
    static uint32_t seed = 0;
    seed++; // محاكاة بسيطة للعشوائية
    return (uint8_t)(seed % 4); 
}

#endif /* HADEED_EMPATHY_H */
