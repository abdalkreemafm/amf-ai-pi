# Hadeed Protocol V1

## Frame layout

All multi-byte integers are **big-endian**. The frame is encoded explicitly; C
struct layout is never transmitted directly.

| Offset | Size | Field |
|---:|---:|---|
| 0 | 1 | SOF `0x5A` |
| 1 | 1 | Version `1` |
| 2 | 1 | Message type |
| 3 | 2 | Payload length |
| 5 | 4 | Sequence |
| 9 | 4 | Timestamp in milliseconds |
| 13 | N | Payload |
| 13+N | 2 | CRC-16/CCITT-FALSE over bytes 1..12+N |

Current message types:

- `0x01`: synchronization/heartbeat.
- `0x02`: vision result.

The maximum payload is 1024 bytes. Receivers must reject unsupported versions,
invalid lengths, invalid SOF, and CRC failures. The current firmware and K210
code still use the legacy frame and must be migrated in a separate integration
change after the hardware build target is confirmed.
