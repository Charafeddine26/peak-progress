# EEPROM Memory Map — Peak Progress

## Arduino Uno WiFi Rev.2 (ATmega4809)

**Total EEPROM:** 256 bytes  
**Rated endurance:** 100,000 write/erase cycles per cell

## Layout

| Address     | Size      | Type         | Content                                    |
| ----------- | --------- | ------------ | ------------------------------------------ |
| `0x00`      | 1 byte    | `uint8_t`    | Magic byte (`0xA5`) — first-boot detection |
| `0x01`      | 2 bytes   | `uint16_t`   | `totalSessionsAllTime` (little-endian)     |
| `0x03`      | 1 byte    | `uint8_t`    | `currentMountainIndex` (0-8)               |
| `0x04`      | 1 byte    | `uint8_t`    | `sessionsOnCurrentMountain` (0-28)         |
| `0x05`      | 1 byte    | `uint8_t`    | `summitsReached`                           |
| `0x06`      | 2 bytes   | `uint16_t`   | `unlockedBitfield` (bit i = mountain i)    |
| `0x08`      | 1 byte    | `uint8_t`    | `currentStreakDays`                        |
| `0x09`      | 1 byte    | `uint8_t`    | `longestStreakDays`                        |
| `0x0A-0x0B` | 2 bytes   | `uint8_t[2]` | Reserved (padding)                         |
| `0x0C-0xFF` | 244 bytes | —            | Free for future use                        |

**Total used: 12 bytes** (4.7% of 256 bytes)

## Write Frequency

- Writes occur only on: `logActivity()`, `summitReached()`, `resetProgress()`
- Expected: 1-7 writes/day → ~2,500/year
- Cells rated for 100,000 cycles → **40 years** of use before wear concern

## Version Migration

If the struct changes in a firmware update, change `EEPROM_MAGIC` from `0xA5` to `0xA6`. On boot, detecting the old magic byte triggers a migration or re-initialization.
