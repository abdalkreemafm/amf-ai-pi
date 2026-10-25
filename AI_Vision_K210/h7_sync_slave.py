"""Hadeed V1 synchronization service for the K210."""
import time
from machine import UART
from hadeed_protocol import MSG_SYNC, MSG_VISION_RESULT, encode
from hadeed_framer import FrameReader


class HadeedSyncSlave:
    def __init__(self, uart_port, baudrate=115200):
        self.uart = uart_port
        self.baudrate = baudrate
        self.reader = FrameReader()
        self.master_timestamp = 0
        self.master_sequence = 0
        self.local_offset = 0
        self.last_sync_tick = time.ticks_ms()
        self.is_synced = False

    def update(self):
        if self.uart.any():
            for message_type, sequence, timestamp_ms, payload in self.reader.feed(self.uart.read()):
                if message_type == MSG_SYNC and len(payload) == 1:
                    now = time.ticks_ms()
                    self.master_timestamp = timestamp_ms
                    self.master_sequence = sequence
                    self.local_offset = timestamp_ms - now
                    self.last_sync_tick = now
                    self.is_synced = True
                    return True
        if time.ticks_diff(time.ticks_ms(), self.last_sync_tick) > 2000:
            self.is_synced = False
        return False

    def get_synced_timestamp(self):
        if not self.is_synced:
            return time.ticks_ms() & 0xFFFFFFFF
        return (time.ticks_ms() + self.local_offset) & 0xFFFFFFFF

    def send_response_to_h7(self, class_id, confidence, inference_ms):
        inference_ms = min(65535, max(0, int(inference_ms)))
        payload = bytes((class_id & 0xFF, int(confidence * 100) & 0xFF,
                         (inference_ms >> 8) & 0xFF, inference_ms & 0xFF))
        frame = encode(MSG_VISION_RESULT, self.master_sequence,
                       self.get_synced_timestamp(), payload)
        self.uart.write(frame)
