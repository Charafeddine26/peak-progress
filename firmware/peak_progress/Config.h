#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ArduinoBLE.h>

// ─── BLE ─────────────────────────────────────────────────────
#define SERVICE_UUID       "19B10000-E8F2-537E-4F6C-D104768A1214"
#define CHAR_PROGRESS_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"
#define CHAR_COMMAND_UUID  "19B10003-E8F2-537E-4F6C-D104768A1214"

#define CMD_LOG_ACTIVITY 0x01
#define CMD_RESET        0x02

BLEService peakService(SERVICE_UUID);
BLECharacteristic progressChar(CHAR_PROGRESS_UUID, BLERead | BLENotify, 8);
BLEByteCharacteristic commandChar(CHAR_COMMAND_UUID, BLEWrite);

// ─── Servo (direct PWM, no library) ─────────────────────────
#define SERVO_PIN   8
#define SERVO_MIN   20
#define SERVO_MAX   170
#define STEP_DELAY  15
int angle = SERVO_MIN;

// ─── Buzzer ─────────────────────────
#define BUZZER_PIN 7

// ─── Mountains ───────────────────────────────────────────────
#define NUM_MTNS       3
#define SESSIONS_PER   3

// mtnUnlock[i] = i (mountain i unlocks after i summits)

// ─── Progress ────────────────────────────────────────────────
#define EEPROM_MAGIC 0xA5

struct Progress {
  uint8_t  mtn;
  uint8_t  sessions;
  uint8_t  summits;
  uint8_t  streak;
  uint8_t  bestStreak;
  uint16_t total;
};

Progress p = {0, 0, 0, 0, 0, 0};

#endif