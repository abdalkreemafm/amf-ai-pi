/**
  ******************************************************************************
  * @file           : hadeed_config.h
  * @brief          : Hardware Tuning & Timing Configuration (The Control Panel)
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * This file controls the Pulse Induction timing, ADC sampling windows, 
  * and hardware safety limits. Handle with care.
  ******************************************************************************
  */

#ifndef HADEED_CONFIG_H
#define HADEED_CONFIG_H

/* --- 1. PULSE INDUCTION (PI) TIMING - التوقيت العسكري للنبضة --- */
// التوقيت بالميكروثانية (Microseconds)

// النمط الافتراضي (Normal Mode)
#define PI_DEFAULT_PULSE_WIDTH_US    150   // عرض النبضة القياسي
#define PI_DEFAULT_PERIOD_MS         10    // التردد (100 هرتز)

// نظام "ناقل الحركة التكيفي" (Adaptive Pulse Gearbox)
#define PI_GEAR_ECO_WIDTH_US         50    // نمط توفير الطاقة
#define PI_GEAR_FOCUS_WIDTH_US       250   // نمط التركيز عند الاشتباه
#define PI_GEAR_HYPER_WIDTH_US       500   // نمط الاختراق الأقصى (Pinpoint)

// زمن التخميد والانتظار (Dead-time/Damping)
#define PI_START_SAMPLING_DELAY_US   15    // الوقت قبل بدء قراءة الـ ADC بعد إغلاق النبضة
#define PI_ACTIVE_DAMPING_TIME_US    8     // وقت تشغيل الـ SSR (AQY210) لامتصاص الصدمة

/* --- 2. ADS1256 ADC CONFIGURATION - إعدادات حاسة السمع الرقمية --- */
#define ADC_VREF_VOLTAGE             2.5f  // الجهد المرجعي الدقيق
#define ADC_DATA_RATE_SPS            30000 // سرعة أخذ العينات (30 ألف عينة/ثانية)
#define ADC_DEFAULT_GAIN             ADS1256_GAIN_64 // الحساسية القصوى (x64)

/* --- 3. SMF SPECTRAL WINDOWS - نوافذ التحليل الترددي --- */
// تحديد أي عينات من الـ 512 عينة تمثل أي تردد
#define SMF_WINDOW_HIGH_START        2     // العينات المبكرة (للأهداف الصغيرة والطبيعي)
#define SMF_WINDOW_HIGH_END          20
#define SMF_WINDOW_MID_START         50    // العينات المتوسطة
#define SMF_WINDOW_MID_END           150
#define SMF_WINDOW_LOW_START         200   // العينات المتأخرة (للأهداف العميقة والصلبة)
#define SMF_WINDOW_LOW_END           450

/* --- 4. SENSOR THRESHOLDS - عتبات الاستشعار والضجيج --- */
#define SIG_NOISE_FLOOR_DEFAULT      15.0f // مستوى الضجيج المقبول
#define SIG_CONFIDENCE_THRESHOLD     85    // النسبة المئوية لإعلان الهدف
#define MAG_ANOMALY_THRESHOLD_NT     1500  // عتبة الشذوذ المغناطيسي (للرانات)

/* --- 5. THERMAL & POWER SAFETY - حماية الهاردوير والبطارية --- */
#define SYS_TEMP_LIMIT_CELSIUS       65.0f // درجة الحرارة القصوى قبل خفض الطاقة
#define SYS_TEMP_COMPENSATION_COEFF  0.02f // معامل تعويض الانحراف الحراري للمكبرات
#define BATT_LOW_THRESHOLD_VOLTS     10.5f // عتبة البطارية المنخفضة (لـ 3S Li-ion)
#define BATT_CRITICAL_SHUTDOWN       9.6f  // الإطفاء الإجباري لحماية الخلايا

/* --- 6. COMMUNICATION TIMers - مواعيد إرسال البيانات --- */
#define BLE_UPDATE_RATE_MS           50    // تحديث التطبيق كل 50 ملي ثانية (20 هرتز)
#define LOGGER_BUFFER_SIZE           100   // عدد الأسطر قبل الكتابة في الـ SD Card

/* --- 7. EMOTIONAL AI TIMING - توقيت المرافق الصوتي --- */
#define EMOTIONAL_SILENCE_DELAY_SEC  600   // (10 دقائق) قبل الدخول في وضع "الصمت العميق"
#define SWING_SPEED_ALERT_LIMIT      1.8f  // سرعة الأرجحة التي تستدعي "تمهل يا صديقي"

#endif /* HADEED_CONFIG_H */
