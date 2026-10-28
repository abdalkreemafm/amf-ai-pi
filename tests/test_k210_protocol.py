import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parents[1] / "AI_Vision_K210"))

from hadeed_framer import FrameReader
from hadeed_protocol import MSG_SYNC, MSG_VISION_RESULT, decode, encode


def test_k210_codec_round_trip():
    raw = encode(MSG_SYNC, 9, 123, b"\x03")
    assert decode(raw) == (MSG_SYNC, 9, 123, b"\x03")


def test_reader_recovers_from_noise_and_partial_frames():
    raw = encode(MSG_VISION_RESULT, 4, 55, b"abc")
    reader = FrameReader()
    assert reader.feed(b"noise" + raw[:5]) == []
    assert reader.feed(raw[5:]) == [(MSG_VISION_RESULT, 4, 55, b"abc")]
