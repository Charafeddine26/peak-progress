/**
 * peak_progress.ino — Peak Progress v2.1
 *
 * Single-file firmware for Arduino Uno WiFi Rev.2.
 * Tangible mountain-climbing habit tracker.
 *
 * Trusts EEPROM calibration on boot. Use serial +/- to jog,
 * f to save floor, H to save top if recalibration is needed.
 *
 * M2 IHM TII — Tangible Interfaces 2026
 */

#include "Config.h"
#include "Ble_manager.h"
#include "Memory.h"
#include "Hardware.h"

bool connectionSoundPlayed = false;

void printStatus() {
  Serial.print(F("A="));
  Serial.print(angle);
  Serial.print(F(" F="));
  Serial.print(cal.floorAngle);
  Serial.print(F(" T="));
  Serial.print(cal.topAngle);
  Serial.print(F(" M="));
  Serial.print(p.mtn);
  Serial.print(F(" S="));
  Serial.print(p.sessions);
  Serial.print('/');
  Serial.print(SESSIONS_FOR(p.mtn));
  Serial.print(F(" U="));
  Serial.print(p.summits);
  Serial.print(F(" X="));
  Serial.println(p.total);
}

void printHelp() {
  Serial.println(F("+/- 1deg  </> 5deg"));
  Serial.println(F("0 floor  9 top"));
  Serial.println(F("f save floor  H save top"));
  Serial.println(F("d defaults  r reset progress"));
  Serial.println(F("p status  h help"));
}

bool saveCurrentAsFloor() {
  if (angle >= cal.topAngle - SERVO_CALIB_MIN_SPAN) {
    Serial.println(F("Floor too close to top."));
    return false;
  }
  cal.floorAngle = angle;
  cal.magic = CALIB_MAGIC;
  saveCalibration();
  Serial.println(F("Floor saved."));
  printStatus();
  return true;
}

bool saveCurrentAsTop() {
  if (angle <= cal.floorAngle + SERVO_CALIB_MIN_SPAN) {
    Serial.println(F("Top too close to floor."));
    return false;
  }
  cal.topAngle = angle;
  cal.magic = CALIB_MAGIC;
  saveCalibration();
  Serial.println(F("Top saved."));
  printStatus();
  return true;
}

void resetCalibrationToDefaults() {
  resetCalibration();
  Serial.println(F("Defaults restored."));
  printStatus();
}

void handleSerialCommand(char c) {
  if (c == '+' || c == '-') {
    moveServoRaw(angle + (c == '+' ? 1 : -1));
    printStatus();
  } else if (c == '>' || c == '<') {
    moveServoRaw(angle + (c == '>' ? 5 : -5));
    printStatus();
  } else if (c == '0') {
    moveServo(cal.floorAngle);
    printStatus();
  } else if (c == '9') {
    moveServo(cal.topAngle);
    printStatus();
  } else if (c == 'f') {
    saveCurrentAsFloor();
  } else if (c == 'H') {
    saveCurrentAsTop();
  } else if (c == 'd') {
    resetCalibrationToDefaults();
  } else if (c == 'r') {
    resetProgress();
    printStatus();
  } else if (c == 'p') {
    printStatus();
  } else if (c == 'h') {
    printHelp();
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  load();

  // Trust EEPROM calibration — move to current progress position
  angle = constrain(targetAngle(), SERVO_HARD_MIN, SERVO_HARD_MAX);
  moveServoRaw(angle);

  printHelp();
  printStatus();

  if (!BLE.begin()){
    while (1){
      Serial.println("BLE Fail");
      delay(1000);
    }
  };

  setBLE();
  updateBLE();
}

void loop() {
  if (Serial.available()) {
    handleSerialCommand(Serial.read());
  }

  BLEDevice central = BLE.central();
  if (central && central.connected()) {
    if (!connectionSoundPlayed) {
      playConnect();
      connectionSoundPlayed = true;
    }
    if (commandChar.written()) {
      uint8_t cmd = commandChar.value();
      if (cmd == CMD_LOG_ACTIVITY) logActivity();
      else if (cmd == CMD_RESET) resetProgress();
    }
  } else {
    connectionSoundPlayed = false;
  }
  BLE.poll();
}
