/**
 * peak_progress.ino — Peak Progress v2.0
 *
 * Single-file firmware for Arduino Uno WiFi Rev.2.
 * Tangible mountain-climbing habit tracker.
 *
 * BLE Characteristics:
 *   Progress (0x0001) — 8 bytes, read/notify
 *   Command  (0x0003) — 1 byte, write (0x01=log, 0x02=reset)
 *
 * M2 IHM TII — Tangible Interfaces 2026
 */

#include <ArduinoBLE.h>
#include <EEPROM.h>

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

// ─── EEPROM ──────────────────────────────────────────────────

void load() {
  if (EEPROM.read(0) != EEPROM_MAGIC) {
    p = {0, 0, 0, 0, 0, 0};
    EEPROM.write(0, EEPROM_MAGIC);
    EEPROM.put(1, p);
    return;
  }
  EEPROM.get(1, p);
  if (p.mtn >= NUM_MTNS) {
    p = {0, 0, 0, 0, 0, 0};
    EEPROM.put(1, p);
  }
}

void save() { EEPROM.put(1, p); }

// ─── Servo Control (direct pulse, no library) ───────────────

void servoPulse(int deg) {
  int us = 544 + ((long)deg * (2400 - 544)) / 180;
  digitalWrite(SERVO_PIN, HIGH);
  delayMicroseconds(us);
  digitalWrite(SERVO_PIN, LOW);
}

void moveServo(int target) {
  target = constrain(target, SERVO_MIN, SERVO_MAX);
  pinMode(SERVO_PIN, OUTPUT);
  int step = (target > angle) ? 1 : -1;
  while (angle != target) {
    angle += step;
    servoPulse(angle);
    delay(STEP_DELAY);
  }
  // Hold position briefly then stop pulsing
  for (uint8_t i = 0; i < 25; i++) {
    servoPulse(angle);
    delay(20);
  }
  digitalWrite(SERVO_PIN, LOW);
}

int targetAngle() {
  return SERVO_MIN + ((long)p.sessions * (SERVO_MAX - SERVO_MIN)) / SESSIONS_PER;
}

// ─── BLE Update ──────────────────────────────────────────────

void updateBLE() {
  uint8_t data[8] = {
    p.mtn, p.sessions, SESSIONS_PER,
    p.summits, p.streak, p.bestStreak,
    (uint8_t)(p.total >> 8), (uint8_t)(p.total & 0xFF)
  };
  progressChar.writeValue(data, 8);
}

// ─── Core Logic ──────────────────────────────────────────────

uint8_t nextMtn() {
  uint8_t next = p.mtn + 1;
  if (next < NUM_MTNS && p.summits >= next) return next;
  return 0;
}

void logActivity() {
  if (p.sessions >= SESSIONS_PER) return;

  p.sessions++;
  p.total++;
  p.streak++;
  if (p.streak > p.bestStreak) p.bestStreak = p.streak;

  moveServo(targetAngle());
  save();

  if (p.sessions >= SESSIONS_PER) {
    delay(3000);
    p.summits++;
    p.mtn = nextMtn();
    p.sessions = 0;
    save();
    moveServo(SERVO_MIN);
  }

  updateBLE();
}

void resetProgress() {
  p = {0, 0, 0, 0, 0, 0};
  save();
  moveServo(SERVO_MIN);
  updateBLE();
}

// ─── Setup & Loop ────────────────────────────────────────────

void setup() {
  load();
  angle = targetAngle();
  moveServo(angle);

  if (!BLE.begin()) while (1);

  BLE.setLocalName("PeakProgress");
  BLE.setAdvertisedService(peakService);
  peakService.addCharacteristic(progressChar);
  peakService.addCharacteristic(commandChar);
  BLE.addService(peakService);
  BLE.advertise();

  updateBLE();
}

void loop() {
  BLEDevice central = BLE.central();
  if (central && central.connected()) {
    if (commandChar.written()) {
      uint8_t cmd = commandChar.value();
      if (cmd == CMD_LOG_ACTIVITY) logActivity();
      else if (cmd == CMD_RESET) resetProgress();
    }
  }
  BLE.poll();
}
