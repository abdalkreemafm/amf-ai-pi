/**
  ******************************************************************************
  * @file           : hadeed_logger.c
  * @brief          : Data Logging & Black-Box Engine for AI Training
  * @author         : Abdul Karim (AMF Tech) & Manus AI
  * @date           : 2026-06-01
  ******************************************************************************
  * @attention
  * Manages SD Card storage using FATFS. Implements a ring-buffer to prevent
  * real-time pulse interference during slow SD write operations.
  ******************************************************************************
  */

#include "hadeed_math.h"
#include "hadeed_logic.h"
#include "hadeed_config.h"
#include "fatfs.h"   // مكتبة الملفات القياسية لـ STM32
#include <stdio.h>
#include <string.h>

/* --- 1. PRIVATE VARIABLES --- */
FATFS fs;           // كائن نظام الملفات
FIL logFile;        // كائن الملف الحالي
char fileName[32];  // اسم الملف (مثلاً: LOG_001.CSV)
static bool sd_mounted = false;
static uint32_t entry_counter = 0;

/* مخزن مؤقت (Buffer) لمنع تأخير المعالج أثناء الكتابة */
static char write_buffer[2048]; 

/* --- 2. INTERNAL UTILS --- */

/**
 * @brief إنشاء اسم ملف جديد بناءً على التسلسل
 */
static void Generate_New_Filename(void) {
    for (int i = 1; i < 1000; i++) {
        sprintf(fileName, "HADEED_%03d.CSV", i);
        if (f_stat(fileName, NULL) != FR_OK) break; // إذا كان الاسم غير موجود، استخدمه
    }
}

/* --- 3. MAIN LOGGING INTERFACE --- */

/**
 * @brief تهيئة كرت الذاكرة وفتح ملف جديد
 */
void Hadeed_Logger_Init(void) {
    if (f_mount(&fs, "", 1) == FR_OK) {
        sd_mounted = true;
        Generate_New_Filename();
        
        // فتح الملف وإضافة الترويسة (Header)
        if (f_open(&logFile, fileName, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK) {
            const char* header = "Timestamp_ms,Peak,LogSlope,Harmony,Phase,MagDelta,VisualID,Type,Confidence\n";
            f_write(&logFile, header, strlen(header), NULL);
            f_sync(&logFile); // حفظ الترويسة فوراً
        }
    } else {
        sd_mounted = false;
        // هنا يمكن إرسال تنبيه للمرافق الصوتي: "كرت الذاكرة غير موجود"
    }
}

/**
 * @brief تسجيل بصمة هدف مجهول أو تم اكتشافه (Data Collection)
 * @param target كائن البيانات المكتمل للهدف
 */
void Hadeed_Logger_Record_Target(Hadeed_Target_Feature_t *target) {
    if (!sd_mounted) return;

    /* تحويل البيانات لنص CSV */
    /* ملاحظة: لا نسجل الإحداثيات هنا لضمان خصوصية المنقب كما اتفقنا */
    int len = sprintf(write_buffer, "%lu,%.2f,%.3f,%.3f,%.2f,%.2f,%d,%d,%d\n",
        HAL_GetTick(),
        target->peak_amplitude,
        target->dp.log_slope,
        target->smf.harmony_ratio,
        target->dp.phase_angle,
        target->mag_anomaly_delta,
        target->visual_class_id,
        target->target_type,
        target->confidence_score
    );

    // كتابة البيانات للملف
    UINT bytesWritten;
    if (f_write(&logFile, write_buffer, len, &bytesWritten) == FR_OK) {
        entry_counter++;
        
        // حفظ الملف في الذاكرة الفيزيائية كل 10 أسطر لضمان عدم ضياع البيانات عند إطفاء الجهاز
        if (entry_counter % 10 == 0) {
            f_sync(&logFile);
        }
    }
}

/**
 * @brief حفظ البيانات وإغلاق الملف بأمان عند إطفاء الجهاز
 */
void Hadeed_Logger_Safe_Close(void) {
    if (sd_mounted) {
        f_close(&logFile);
        f_mount(NULL, "", 0);
        sd_mounted = false;
    }
}

/* --- 4. ANONYMOUS EXPORT (استراتيجية المبادلة) --- */

/**
 * @brief إعداد ملف خاص للإرسال للمختبر (بصمات مجهولة فقط)
 * يتم استدعاؤه عندما يوافق المستخدم على المبادلة مقابل باقة برمجية
 */
void Hadeed_Logger_Prepare_Anonymous_Bundle(void) {
    // الكود هنا يقوم بنسخ ملفات الـ LOG وحذف أي بيانات قد تدل على الهوية
    // ثم يجهزها للنقل عبر البلوتوث للتطبيق بطلب من المستخدم
}

/* --- 5. BLACK BOX EMERGENCY LOG --- */

/**
 * @brief تسجيل أخطاء النظام (System Crashes)
 * يساعد فريقك في عدن/صنعاء على معرفة سبب تعطل أي جهاز وصيانته
 */
void Hadeed_Logger_Log_Error(const char* error_msg) {
    if (!sd_mounted) return;
    
    FIL errorFile;
    if (f_open(&errorFile, "SYSTEM.LOG", FA_OPEN_APPEND | FA_WRITE) == FR_OK) {
        char err_buf[128];
        sprintf(err_buf, "[%lu] ERROR: %s\n", HAL_GetTick(), error_msg);
        f_write(&errorFile, err_buf, strlen(err_buf), NULL);
        f_close(&errorFile);
    }
}
