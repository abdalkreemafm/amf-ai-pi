"""Hadeed wire protocol V1.

The wire format is deliberately explicit; no language ABI or struct padding is
allowed on the wire.
"""
from __future__ import annotations

import struct
from dataclasses import dataclass

SOF = 0x5A
VERSION = 1
MSG_SYNC = 0x01
MSG_VISION_RESULT = 0x02
MAX_PAYLOAD = 1024
_HEADER = struct.Struct(">BBB H I I")


class ProtocolError(ValueError):
    """Raised when a frame is malformed or fails integrity validation."""


def crc16_ccitt(data: bytes, crc: int = 0xFFFF) -> int:
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


@dataclass(frozen=True)
class Frame:
    message_type: int
    sequence: int
    timestamp_ms: int
    payload: bytes = b""
    version: int = VERSION

    def encode(self) -> bytes:
        if not 0 <= self.message_type <= 0xFF:
            raise ProtocolError("message_type must fit in one byte")
        if not 0 <= self.sequence <= 0xFFFFFFFF or not 0 <= self.timestamp_ms <= 0xFFFFFFFF:
            raise ProtocolError("sequence and timestamp must be uint32")
        if len(self.payload) > MAX_PAYLOAD:
            raise ProtocolError("payload exceeds MAX_PAYLOAD")
        header = _HEADER.pack(SOF, self.version, self.message_type, len(self.payload), self.sequence, self.timestamp_ms)
        body = header[1:] + self.payload
        return header + self.payload + struct.pack(">H", crc16_ccitt(body))


def decode(data: bytes) -> Frame:
    if len(data) < _HEADER.size + 2:
        raise ProtocolError("frame is truncated")
    sof, version, message_type, payload_length, sequence, timestamp_ms = _HEADER.unpack_from(data)
    if sof != SOF:
        raise ProtocolError("invalid SOF")
    if version != VERSION:
        raise ProtocolError(f"unsupported protocol version: {version}")
    if payload_length > MAX_PAYLOAD:
        raise ProtocolError("payload exceeds MAX_PAYLOAD")
    expected_size = _HEADER.size + payload_length + 2
    if len(data) != expected_size:
        raise ProtocolError("frame length does not match payload length")
    payload_end = _HEADER.size + payload_length
    received_crc = struct.unpack_from(">H", data, payload_end)[0]
    calculated_crc = crc16_ccitt(data[1:payload_end])
    if received_crc != calculated_crc:
        raise ProtocolError("CRC mismatch")
    return Frame(message_type, sequence, timestamp_ms, data[_HEADER.size:payload_end], version)
