/**
  ******************************************************************************
  * @file           : hadeed_state_machine.c
  * @brief          : Sentient State Machine (Awareness & Context Engine)
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * Manages transitions between Stealth, Alert, and Investigation modes.
  * Implements the "Auto-Pilot" philosophy with zero user intervention.
  ******************************************************************************
  */

#include "hadeed_math.h"
#include "hadeed_logic.h"
#include "hadeed_config.h"
#include "hadeed_sync.h"

/* --- 1. PRIVATE VARIABLES --- */
static uint32_t state_timer_start = 0;
static bool investigation_locked = false;

/* --- 2. INTERNAL HELPER FUNCTIONS --- */

/**
 * @brief الحصول على الحالة الحالية للجهاز
 */
static void Update_System_State(Hadeed_System_State_t new_state) {
    extern Hadeed_System_Health_t g_sys_health;
    if (g_sys_health.current_state != new_state) {
        g_sys_health.current_state = new_state;
        state_timer_start = HAL_GetTick(); // إعادة ضبط مؤقت الحالة
        
        // إبلاغ المساعد الصوتي بتغيير الطور إذا لزم الأمر
        if (new_state == STATE_ENVIRONMENT_SCAN) Hadeed_Notify_User(VOICE_CMD_WELCOME);
    }
}

/* --- 3. MAIN STATE MACHINE ENGINE --- */

/**
 * @brief المحرك الرئيسي لتطوير الوعي وتغيير الأطوار
 * @param target مؤشر لبيانات الهدف الحالي
 * @param health مؤشر لحالة النظام
 */
void Hadeed_State_Machine_Update(Hadeed_Target_Feature_t *target, Hadeed_System_Health_t *health) {
    uint32_t current_time = HAL_GetTick();

    switch (health->current_state) {
        
        case STATE_BOOTING:
            // فحص الحساسات والتعرف على الكويل (تم في main)
            if (health->sensor_ads1256_ok && health->sensor_rm3100_ok) {
                Update_System_State(STATE_ENVIRONMENT_SCAN);
            } else {
                Update_System_State(STATE_EMERGENCY_RECOVERY);
            }
            break;

        case STATE_ENVIRONMENT_SCAN:
            // أول 5 ثوانٍ: صمت تام لدراسة بصمة الأرض الكيميائية والمغناطيسية
            if ((current_time - state_timer_start) > 5000) {
                Hadeed_Notify_User(VOICE_CMD_SOIL_READY);
                Update_System_State(STATE_STEALTH_SCAN);
            }
            // خلال هذه الفترة، الموازنة الأرضية تعمل بأقصى سرعة تعلم
            Hadeed_Enable_Fast_Learning(true);
            break;

        case STATE_STEALTH_SCAN:
            // الطور الشبح: الجهاز صامت، يستهلك أقل طاقة (Gear: ECO)
            Hadeed_Set_Pulse_Gear(1); 
            
            // شرط الاستيقاظ: إذا تجاوزت الإشارة 1.5 ضعف مستوى الضجيج
            if (target->peak_amplitude > (SIG_NOISE_FLOOR_DEFAULT * 1.5f)) {
                Update_System_State(STATE_ALERT_SENSING);
            }
            break;

        case STATE_ALERT_SENSING:
            // طور التحفز: الجهاز رصد "ذيل" إشارة (Gear: FOCUS)
            Hadeed_Set_Pulse_Gear(2);
            
            // إذا استقرت الإشارة وأصبحت واضحة (ثقة > 70%)
            if (target->confidence_score > CONFIDENCE_LEVEL_MEDIUM) {
                // إبلاغ المنقب بلطف (توجيه عاطفي)
                if (target->target_type == TARGET_ID_TREASURE_GOLD) {
                    Hadeed_Notify_User(VOICE_CMD_GOLD_DEEP);
                }
            }

            // العودة للصمت إذا تلاشت الإشارة (المقيد ابتعد عن الهدف)
            if (target->peak_amplitude < SIG_NOISE_FLOOR_DEFAULT) {
                Update_System_State(STATE_STEALTH_SCAN);
            }

            // التحول للتحقيق العميق إذا ضغط المستخدم الزر أو ثبت الكويل
            if (Hadeed_Is_Pinpoint_Button_Pressed()) {
                Update_System_State(STATE_DEEP_INVESTIGATE);
            }
            break;

        case STATE_DEEP_INVESTIGATE:
            // طور التحقيق: أقصى طاقة (Gear: HYPER)، تصوير 3D مفعل
            Hadeed_Set_Pulse_Gear(3);
            
            // طلب فتح تطبيق الهاتف لرؤية المجسم
            if (!investigation_locked) {
                Hadeed_Notify_User(VOICE_CMD_APP_OPEN_3D);
                investigation_locked = true;
            }

            // العودة لطور التنبيه عند ترك الزر
            if (!Hadeed_Is_Pinpoint_Button_Pressed()) {
                investigation_locked = false;
                Update_System_State(STATE_ALERT_SENSING);
            }
            break;

        case STATE_EMERGENCY_RECOVERY:
            // طور الطوارئ: محاولة إعادة تشغيل الحساسات صمتاً
            Hadeed_Attempt_Internal_Reset();
            if (health->sensor_ads1256_ok) {
                Update_System_State(STATE_STEALTH_SCAN);
            }
            break;

        case STATE_POWER_ADVISOR:
            // طور الحفاظ على البطارية: العمل بالحد الأدنى من الحواس
            Hadeed_Set_Pulse_Gear(1);
            Hadeed_Disable_Vision_Module(); // إطفاء الكاميرا لتوفير الطاقة
            break;
    }
}

/**
 * @brief التحكم في "ناقل الحركة التكيفي" للنبضة
 * @param gear الترس المطلوب (1: ECO, 2: FOCUS, 3: HYPER)
 */
void Hadeed_Set_Pulse_Gear(uint8_t gear) {
    extern Hadeed_Coil_Profile_t g_current_coil;
    uint16_t new_width = Hadeed_Get_Optimal_Pulse_Width(&g_current_coil, gear);
    
    // تحديث عرض النبضة في المعالج
    Hadeed_PWM_Update_Width(new_width);
    
    // تحديث سرعة المزامنة مع K210
    if (gear == 3) Hadeed_Sync_Set_High_Speed(true);
    else Hadeed_Sync_Set_High_Speed(false);
}
