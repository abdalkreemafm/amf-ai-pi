# ******************************************************************************
# * @file           : soil_classifier.py
# * @brief          : AI Vision Engine for Rock & Soil Classification
# * @author         : Abdul Karim (AMF Tech) & Manus AI
# * @device         : Kendryte K210 (Sipeed Maix Bit/Dock)
# ******************************************************************************
# * @attention
# * Runs a TinyML KModel to identify "Hot Rocks" visually.
# * Syncs with STM32H7 via UART to ensure temporal alignment.
# ******************************************************************************

import sensor, image, lcd, time
import KPU as kpu
from machine import UART
from fpioa_manager import fm
from hadeed_protocol import MSG_SYNC, MSG_VISION_RESULT, encode
from hadeed_framer import FrameReader

# --- 1. CONFIGURATION & PROTOCOL ---
UART_PORT = UART.UART1
BAUD_RATE = 115200

# تعريف مسميات التربة (تطابق الاوزان في النموذج)
LABELS = ["Normal_Soil", "Sand", "Clay", "Quartz", "Mineralized", "Basalt_HotRock"]

# --- 2. HARDWARE INITIALIZATION ---
# تهيئة الشاشة (للمعاينة الميدانية)
lcd.init(freq=15000000)
lcd.rotation(2) # تعديل الاتجاه حسب تركيب الكاميرا

# تهيئة الكاميرا (OV2640)
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA) # 320x240
sensor.set_vflip(1)
sensor.run(1)

# تهيئة منفذ التواصل مع STM32H7
fm.register(34, fm.fpioa.UART1_TX, force=True)
fm.register(35, fm.fpioa.UART1_RX, force=True)
uart = UART(UART.UART1, BAUD_RATE, 8, 1, 0, timeout=10, read_buf_len=4096)

# --- 3. AI MODEL LOADING ---
# تحميل نموذج الذكاء الاصطناعي من كرت الذاكرة
# المسار المتوقع للموديل: /sd/soil_analysis.kmodel
try:
    task = kpu.load("/sd/soil_analysis.kmodel")
    # ضبط العتبة (Threshold) لتقليل القرارات الخاطئة
    kpu.set_outputs(task, 0, 1, 1, 6) # 6 فئات تصنيف
except:
    print("Error: AI Model not found on SD card!")

# --- 4. DATA PACKET ENGINE ---

def send_result_to_h7(class_id, confidence, master_ts, sequence, inference_ms):
    """
    إرسال النتيجة بتنسيق باكت منظم لـ STM32H7
    V1 payload: [Class_ID, Confidence_0_to_100, Inference_ms_u16]
    """
    inference_ms = min(65535, max(0, int(inference_ms)))
    payload = bytes((class_id & 0xFF, int(confidence * 100) & 0xFF,
                     (inference_ms >> 8) & 0xFF, inference_ms & 0xFF))
    uart.write(encode(MSG_VISION_RESULT, sequence, master_ts, payload))

# --- 5. MAIN AI LOOP ---
master_timestamp = 0
master_sequence = 0
frame_reader = FrameReader()

while True:
    # أ. التحقق من وجود نبضة مزامنة (Heartbeat) قادمة من H7
    if uart.any():
        for message_type, sequence, timestamp_ms, payload in frame_reader.feed(uart.read()):
            if message_type == MSG_SYNC and len(payload) == 1:
                master_sequence = sequence
                master_timestamp = timestamp_ms

    # ب. التقاط صورة ومعالجتها
    img = sensor.snapshot()
    
    # ج. تنفيذ الاستدلال (Inference)
    # ملاحظة: النموذج مدرب على التعرف على "أنسجة الصخور"
    inference_started = time.ticks_ms()
    fmap = kpu.forward(task, img)
    plist = fmap[:]
    pmax = max(plist)
    max_id = plist.index(pmax)

    # د. عرض النتيجة على الشاشة الصغيرة للجهاز (للمنقب)
    img.draw_string(10, 10, "AI Vision: %s" % (LABELS[max_id]), color=(255, 255, 0), scale=2)
    img.draw_string(10, 40, "Conf: %2.1f%%" % (pmax * 100), color=(0, 255, 0), scale=2)
    lcd.display(img)

    # هـ. إرسال القرار فوراً للمعالج STM32H7 لدمجه مع إشارة النبض
    if pmax > 0.75: # لا نرسل القرار إلا إذا كانت الثقة أكبر من 75%
        elapsed = time.ticks_diff(time.ticks_ms(), inference_started)
        send_result_to_h7(max_id, pmax, master_timestamp, master_sequence, elapsed)

    # و. تبريد المعالج قليلاً لمنع الحرارة
    time.sleep_ms(50)

# تنظيف المصادر (في حال الخروج)
kpu.deinit(task)
