#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[1]
required = [
    root / "protocol/hadeed_protocol.c",
    root / "protocol/hadeed_protocol.h",
    root / "CMakeLists.txt",
    root / "Mobile_App/pubspec.yaml",
    root / "Mobile_App/lib/main.dart",
    root / "hardware/stm32/Hadeed.ioc.template",
]
missing = [str(path.relative_to(root)) for path in required if not path.is_file()]
if missing:
    raise SystemExit("missing scaffold files: " + ", ".join(missing))
print("project scaffolding: OK")
