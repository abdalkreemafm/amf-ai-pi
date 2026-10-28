import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parents[1]))

import pytest
from protocol.hadeed_protocol import Frame, MSG_SYNC, ProtocolError, decode


def test_round_trip():
    frame = Frame(MSG_SYNC, 42, 123456, b"hadeed")
    assert decode(frame.encode()) == frame


def test_known_crc_vector():
    frame = Frame(MSG_SYNC, 1, 2, b"")
    assert frame.encode().hex() == "5a010100000000000100000002d5d6"


def test_rejects_corruption():
    data = bytearray(Frame(MSG_SYNC, 1, 2, b"abc").encode())
    data[-1] ^= 0x01
    with pytest.raises(ProtocolError, match="CRC"):
        decode(bytes(data))


def test_rejects_trailing_bytes():
    data = Frame(MSG_SYNC, 1, 2, b"abc").encode() + b"x"
    with pytest.raises(ProtocolError, match="length"):
        decode(data)
