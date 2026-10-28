"""Small allocation-conscious Hadeed Protocol V1 codec for MicroPython."""
import struct

SOF = 0x5A
VERSION = 1
MSG_SYNC = 0x01
MSG_VISION_RESULT = 0x02
MAX_PAYLOAD = 1024


def crc16_ccitt(data):
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


def encode(message_type, sequence, timestamp_ms, payload=b""):
    if len(payload) > MAX_PAYLOAD:
        raise ValueError("payload too large")
    header = struct.pack(">BBBHII", SOF, VERSION, message_type, len(payload), sequence, timestamp_ms)
    crc = crc16_ccitt(header[1:] + payload)
    return header + payload + struct.pack(">H", crc)


def decode(frame):
    if len(frame) < 15 or frame[0] != SOF or frame[1] != VERSION:
        raise ValueError("invalid frame header")
    message_type = frame[2]
    payload_len = struct.unpack(">H", frame[3:5])[0]
    expected = 15 + payload_len
    if payload_len > MAX_PAYLOAD or len(frame) != expected:
        raise ValueError("invalid frame length")
    if struct.unpack(">H", frame[-2:])[0] != crc16_ccitt(frame[1:-2]):
        raise ValueError("CRC mismatch")
    sequence = struct.unpack(">I", frame[5:9])[0]
    timestamp_ms = struct.unpack(">I", frame[9:13])[0]
    return message_type, sequence, timestamp_ms, frame[13:-2]
