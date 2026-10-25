"""Incremental UART frame reader for MicroPython."""
from hadeed_protocol import SOF, MAX_PAYLOAD, decode


class FrameReader:
    def __init__(self, max_buffer=1100):
        self.buffer = bytearray()
        self.max_buffer = max_buffer

    def feed(self, data):
        if data:
            self.buffer.extend(data)
        frames = []
        while True:
            try:
                start = self.buffer.index(SOF)
            except ValueError:
                self.buffer = self.buffer[-1:]
                break
            if start:
                del self.buffer[:start]
            if len(self.buffer) < 15:
                break
            payload_len = (self.buffer[3] << 8) | self.buffer[4]
            if payload_len > MAX_PAYLOAD:
                del self.buffer[0]
                continue
            frame_len = 15 + payload_len
            if len(self.buffer) < frame_len:
                break
            candidate = bytes(self.buffer[:frame_len])
            del self.buffer[:frame_len]
            try:
                frames.append(decode(candidate))
            except ValueError:
                # Drop one byte and search for the next SOF.
                if candidate[1:] :
                    self.buffer = bytearray(candidate[1:]) + self.buffer
        if len(self.buffer) > self.max_buffer:
            self.buffer = self.buffer[-self.max_buffer:]
        return frames
