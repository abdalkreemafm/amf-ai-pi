# ******************************************************************************
# * @file           : h7_sync_slave.py
# * @brief          : High-Precision Time Synchronization Slave for K210
# * @author         : Abdul Karim (AMF Tech) & Manus AI
# * @device         : Kendryte K210
# ******************************************************************************
# * @attention
# * This module handles the binary handshake and clock offset calculation
# * between the Master (STM32H7) and the Vision Processor.
# ******************************************************************************

import time
import struct
from machine import UART

class HadeedSyncSlave:
    def __init__(self, uart_port, baudrate=115200):
        self.uart = uart_port
        self.sof = 0x5A
        self.master_time = 0
        self.local_offset = 0
        self.last_sync_tick = 0
        self.is_synced = False
        self.drift_threshold_ms = 5

    def update(self):
        """
        يفحص المنفذ لاستلام حزمة المزامنة ومعالجة البيانات
        Structure: [SOF(1), Master_TS(4), Seq_ID(4), State(1), Checksum(2)] = 12 bytes
        """
        if self.uart.any() >= 12:
            data = self.uart.read(12)
            
            # 1. التحقق من رمز البداية (Start of Frame)
            if data[0] != self.sof:
                return False

            # 2. فك تشفير الحزمة الثنائية (Binary Unpacking)
            # الصيغة: >BIIBH (Big-endian: Byte, Uint32, Uint32, Byte, Uint16)
            try:
                sof, m_ts, seq, state, cksum = struct.unpack(">BIIBH", data)
            except:
                return False

            # 3. التحقق من سلامة البيانات (Checksum)
            calc_cksum = sum(data[:10]) & 0xFFFF
            if calc_cksum != cksum:
                return False

            # 4. حساب الفارق الزمني (Clock Offset)
            # نأخذ التوقيت المحلي الحالي في لحظة الوصول
            local_now = time.ticks_ms()
            self.master_time = m_ts
            self.local_offset = m_ts - local_now
            self.last_sync_tick = local_now
            self.is_synced = True
            
            return True
        
        # إذا مر أكثر من 2 ثانية بدون نبضة مزامنة، نفقد القفل الزمني
        if time.ticks_diff(time.ticks_ms(), self.last_sync_tick) > 2000:
            self.is_synced = False
            
        return False

    def get_synced_timestamp(self):
        """
        إعطاء التوقيت الحالي "بصيغة المعالج الرئيسي"
        هذا هو التوقيت الذي يوضع على نتائج الذكاء الاصطناعي
        """
        if not self.is_synced:
            return time.ticks_ms() # العودة للتوقيت المحلي كخيار احتياطي
        
        return time.ticks_ms() + self.local_offset

    def send_response_to_h7(self, class_id, inference_ms):
        """
        إرسال رد المزامنة + نتيجة الذكاء الاصطناعي لـ STM32H7
        Structure: [SOF(1), Slave_TS(4), Class_ID(1), Inf_Time(4), Checksum(2)]
        """
        slave_ts = self.get_synced_timestamp()
        # تحويل وقت الاستدلال لـ Fixed point (float to int * 100)
        inf_time_fixed = int(inference_ms * 100)
        
        packet = struct.pack(">BIIBH", 
                             self.sof, 
                             slave_ts, 
                             class_id, 
                             inf_time_fixed, 
                             0) # مكان الصفر للتشيك سوم
        
        # حساب الـ Checksum للحزمة الصادرة
        raw_pkt = bytearray(packet)
        cksum = sum(raw_pkt[:10]) & 0xFFFF
        struct.pack_into(">H", raw_pkt, 10, cksum)
        
        self.uart.write(raw_pkt)

# --- مثال على الاستخدام داخل الكود الرئيسي ---
# sync = HadeedSyncSlave(uart)
# if sync.update():
#     print("Clock Synced with STM32H7!")
