# BLE Protocol Specification — Peak Progress

## Service UUID

`19B10000-E8F2-537E-4F6C-D104768A1214`

## Characteristics

### Progress (0x0001) — Read + Notify, 8 bytes

| Byte | Field                            | Range      |
| ---- | -------------------------------- | ---------- |
| 0    | `mountainIndex`                  | 0-8        |
| 1    | `sessionsOnMountain`             | 0-28       |
| 2    | `totalSessionsForMountain`       | 7/14/21/28 |
| 3    | `summitsReached`                 | 0-255      |
| 4    | `currentStreakDays`              | 0-255      |
| 5    | `longestStreakDays`              | 0-255      |
| 6    | `totalSessionsAllTime` high byte | 0-255      |
| 7    | `totalSessionsAllTime` low byte  | 0-255      |

### Mountain (0x0002) — Read + Notify, 20 bytes

Null-terminated UTF-8 string containing the current mountain name (e.g. `"Mont Blanc\0"`).

### Command (0x0003) — Write, 1 byte

| Value  | Command                               |
| ------ | ------------------------------------- |
| `0x01` | Log Activity (same as physical tap)   |
| `0x02` | Reset All Progress                    |
| `0x03` | Select Mountain (reserved for future) |

### Unlock (0x0004) — Read + Notify, 2 bytes

Little-endian 16-bit bitfield. Bit `i` = mountain `i` is unlocked.

Example: `0x0007` = bits 0, 1, 2 set = first 3 mountains unlocked.
