/**
 * ble_test.ino — Peak Progress firmware (no Circuit Playground)
 *
 * Full functionality: BLE + servo + EEPROM + mountain progression.
 * Equivalent to arduino_main with HAS_CIRCUIT_PLAYGROUND disabled.
 * Upload via Arduino IDE with ArduinoBLE installed.
 */

#include <ArduinoBLE.h>
#include <EEPROM.h>
#include <Servo.h>

// ─── BLE ─────────────────────────────────────────────────────
#define SERVICE_UUID       "19B10000-E8F2-537E-4F6C-D104768A1214"
#define CHAR_PROGRESS_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"
#define CHAR_MOUNTAIN_UUID "19B10002-E8F2-537E-4F6C-D104768A1214"
#define CHAR_COMMAND_UUID  "19B10003-E8F2-537E-4F6C-D104768A1214"
#define CHAR_UNLOCK_UUID   "19B10004-E8F2-537E-4F6C-D104768A1214"

#define CMD_LOG_ACTIVITY 0x01
#define CMD_RESET        0x02

BLEService peakService(SERVICE_UUID);
BLECharacteristic progressChar(CHAR_PROGRESS_UUID, BLERead | BLENotify, 8);
BLECharacteristic mountainChar(CHAR_MOUNTAIN_UUID, BLERead | BLENotify, 20);
BLECharacteristic commandChar(CHAR_COMMAND_UUID, BLEWrite, 1);
BLECharacteristic unlockChar(CHAR_UNLOCK_UUID, BLERead | BLENotify, 2);

// ─── Servo ───────────────────────────────────────────────────
#define SERVO_PIN 8
#define SERVO_MIN 20
#define SERVO_MAX 170
#define SERVO_STEP_DELAY 15
Servo climberServo;
int currentAngle = SERVO_MIN;

// ─── Progress ────────────────────────────────────────────────
#define EEPROM_MAGIC      0xA5
#define NUM_MOUNTAINS     3
#define SESSIONS_PER_MTN  3  // Fast test mode

struct Progress {
  uint8_t mountain;
  uint8_t sessions;
  uint8_t summits;
  uint8_t streakDays;
  uint8_t longestStreak;
  uint16_t total;
};

Progress prog = {0, 0, 0, 0, 0, 0};

const char* mtnNames[] = {
  "Colline Locale", "Petit Sommet", "Mont Entrainement"
};
const uint8_t mtnUnlockAfter[] = {0, 1, 2};

// ─── Functions ───────────────────────────────────────────────

void loadProgress() {
  if (EEPROM.read(0) != EEPROM_MAGIC) {
    prog = {0, 0, 0, 0, 0, 0};
    EEPROM.write(0, EEPROM_MAGIC);
    EEPROM.put(1, prog);
    return;
  }
  EEPROM.get(1, prog);
  if (prog.mountain >= NUM_MOUNTAINS) {
    prog = {0, 0, 0, 0, 0, 0};
    EEPROM.put(1, prog);
  }
}

void saveProgress() {
  EEPROM.put(1, prog);
}

uint16_t calcUnlockBitfield() {
  uint16_t unlocked = 0;
  for (uint8_t i = 0; i < NUM_MOUNTAINS; i++) {
    if (prog.summits >= mtnUnlockAfter[i]) unlocked |= (1 << i);
  }
  return unlocked;
}

void updateBLE() {
  // Progress: 8 bytes
  uint8_t data[8] = {
    prog.mountain, prog.sessions, SESSIONS_PER_MTN,
    prog.summits, prog.streakDays, prog.longestStreak,
    (uint8_t)(prog.total >> 8), (uint8_t)(prog.total & 0xFF)
  };
  progressChar.writeValue(data, 8);

  // Mountain name: 20-byte string
  uint8_t nameBuf[20];
  memset(nameBuf, 0, 20);
  strncpy((char*)nameBuf, mtnNames[prog.mountain], 19);
  mountainChar.writeValue((const uint8_t*)nameBuf, 20);

  // Unlock bitfield: 2 bytes
  uint16_t unlocked = calcUnlockBitfield();
  uint8_t unlockData[2] = {
    (uint8_t)(unlocked & 0xFF),
    (uint8_t)(unlocked >> 8)
  };
  unlockChar.writeValue(unlockData, 2);
}

void moveServoSmooth(int target) {
  climberServo.attach(SERVO_PIN);
  target = constrain(target, SERVO_MIN, SERVO_MAX);

  int step = (target > currentAngle) ? 1 : -1;
  while (currentAngle != target) {
    currentAngle += step;
    climberServo.write(currentAngle);
    delay(SERVO_STEP_DELAY);
  }
  delay(500);
  climberServo.detach();
}

int calcAngle() {
  return SERVO_MIN + ((long)prog.sessions * (SERVO_MAX - SERVO_MIN)) / SESSIONS_PER_MTN;
}

uint8_t findNextMountain() {
  uint16_t unlocked = calcUnlockBitfield();
  uint8_t next = prog.mountain + 1;
  if (next < NUM_MOUNTAINS && (unlocked & (1 << next))) {
    return next;
  }
  return 0; // Wrap to first mountain
}

void logActivity() {
  if (prog.sessions >= SESSIONS_PER_MTN) return;

  prog.sessions++;
  prog.total++;
  prog.streakDays++;
  if (prog.streakDays > prog.longestStreak) {
    prog.longestStreak = prog.streakDays;
  }

  moveServoSmooth(calcAngle());
  saveProgress();

  Serial.print("Session ");
  Serial.print(prog.sessions);
  Serial.print("/");
  Serial.print(SESSIONS_PER_MTN);
  Serial.print(" on ");
  Serial.println(mtnNames[prog.mountain]);

  if (prog.sessions >= SESSIONS_PER_MTN) {
    Serial.println("SUMMIT!");
    delay(3000);

    prog.summits++;
    uint8_t nextIdx = findNextMountain();

    if (nextIdx != prog.mountain) {
      Serial.print("New mountain: ");
      Serial.println(mtnNames[nextIdx]);
      delay(2000);
    }

    prog.mountain = nextIdx;
    prog.sessions = 0;
    saveProgress();
    moveServoSmooth(SERVO_MIN);
  }

  updateBLE();
}

void resetProgress() {
  prog = {0, 0, 0, 0, 0, 0};
  saveProgress();
  moveServoSmooth(SERVO_MIN);
  updateBLE();
  Serial.println("Progress reset!");
}

// ─── Setup & Loop ────────────────────────────────────────────

void setup() {
  Serial.begin(9600);
  delay(500);

  loadProgress();
  currentAngle = calcAngle();
  moveServoSmooth(currentAngle);

  // BLE
  if (!BLE.begin()) {
    Serial.println("BLE FAILED!");
    while (1);
  }

  BLE.setLocalName("PeakProgress");
  BLE.setAdvertisedService(peakService);
  peakService.addCharacteristic(progressChar);
  peakService.addCharacteristic(mountainChar);
  peakService.addCharacteristic(commandChar);
  peakService.addCharacteristic(unlockChar);
  BLE.addService(peakService);
  BLE.advertise();

  updateBLE();

  Serial.println("Ready!");
  Serial.print("Mountain: ");
  Serial.println(mtnNames[prog.mountain]);
  Serial.print("Sessions: ");
  Serial.print(prog.sessions);
  Serial.print("/");
  Serial.println(SESSIONS_PER_MTN);
}

void loop() {
  BLEDevice central = BLE.central();

  if (central && central.connected()) {
    if (commandChar.written()) {
      uint8_t cmd = commandChar.value()[0];
      if (cmd == CMD_LOG_ACTIVITY) logActivity();
      else if (cmd == CMD_RESET) resetProgress();
    }
  }

  BLE.poll();
}
