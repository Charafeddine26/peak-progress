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
#define SERVO_PIN              8
#define SERVO_DEFAULT_MIN      20
#define SERVO_DEFAULT_MAX      170
#define SERVO_HARD_MIN         0
#define SERVO_HARD_MAX         180
#define SERVO_CALIB_MIN_SPAN   10
#define STEP_DELAY             15
int angle = SERVO_DEFAULT_MIN;

// ─── Buzzer ─────────────────────────
#define BUZZER_PIN 7

// ─── Mountains ───────────────────────────────────────────────
#define NUM_MTNS 9

// Sessions required per mountain (mirrors mobile app)
const uint8_t MTN_SESSIONS[NUM_MTNS] = {
  7,   // Colline Locale
  7,   // Petit Sommet
  7,   // Mont d'Entrainement
  14,  // Mont Blanc
  14,  // Matterhorn
  21,  // Kilimanjaro
  21,  // Denali
  28,  // Everest
  28   // K2
};

#define SESSIONS_FOR(idx) (MTN_SESSIONS[(idx)])

// mtnUnlock[i] = i (mountain i unlocks after i summits)

// ─── Progress ────────────────────────────────────────────────
#define EEPROM_MAGIC 0xA5
#define CALIB_MAGIC  0x5C

struct Progress {
  uint8_t  mtn;
  uint8_t  sessions;
  uint8_t  summits;
  uint8_t  streak;
  uint8_t  bestStreak;
  uint16_t total;
};

Progress p = {0, 0, 0, 0, 0, 0};

struct Calibration {
  uint8_t magic;
  uint8_t floorAngle;
  uint8_t topAngle;
};

Calibration cal = {CALIB_MAGIC, SERVO_DEFAULT_MIN, SERVO_DEFAULT_MAX};

#define EEPROM_PROGRESS_ADDR 1
#define EEPROM_CALIB_ADDR    (EEPROM_PROGRESS_ADDR + sizeof(Progress))

#endif
