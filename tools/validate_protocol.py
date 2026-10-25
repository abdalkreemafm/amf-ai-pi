#!/usr/bin/env python3
"""Small dependency-free smoke check for environments without pytest."""
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).parents[1]))
from protocol.hadeed_protocol import Frame, MSG_SYNC, decode

frame = Frame(MSG_SYNC, 7, 1000, b"smoke")
assert decode(frame.encode()) == frame
print("protocol smoke test: OK")
