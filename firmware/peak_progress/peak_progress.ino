/**
 * peak_progress.ino — Peak Progress v2.0
 *
 * Single-file firmware for Arduino Uno WiFi Rev.2.
 * Tangible mountain-climbing habit tracker.
 *
 * M2 IHM TII — Tangible Interfaces 2026
 */

#include "Config.h"
#include "Ble_manager.h"
#include "Memory.h"
#include "Hardware.h"

bool connectionSoundPlayed = false;
bool homed = !START_IN_HOMING_MODE;

void printStatus() {
  Serial.print(F("Angle="));
  Serial.print(angle);
  Serial.print(F(" homed="));
  Serial.print(homed ? F("yes") : F("no"));
  Serial.print(F(" floor="));
  Serial.print(cal.floorAngle);
  Serial.print(F(" top="));
  Serial.print(cal.topAngle);
  Serial.print(F(" mtn="));
  Serial.print(p.mtn);
  Serial.print(F(" sessions="));
  Serial.print(p.sessions);
  Serial.print(F("/"));
  Serial.print(SESSIONS_FOR(p.mtn));
  Serial.print(F(" summits="));
  Serial.print(p.summits);
  Serial.print(F(" total="));
  Serial.println(p.total);
}

void printCalibrationHelp() {
  Serial.println(F("Calibration commands:"));
  Serial.println(F("  b     : assume current position is saved floor"));
  Serial.println(F("  m     : assume current position is mid travel"));
  Serial.println(F("  u     : assume current position is saved top"));
  Serial.println(F("  + / - : nudge 1 deg"));
  Serial.println(F("  > / < : nudge 5 deg"));
  Serial.println(F("  0     : move to saved floor"));
  Serial.println(F("  9     : move to saved top"));
  Serial.println(F("  f     : save current angle as floor"));
  Serial.println(F("  t     : save current angle as top"));
  Serial.println(F("  H     : set current position as home and reset progress"));
  Serial.println(F("  d     : reset saved floor/top to defaults"));
  Serial.println(F("  r     : reset progress and go to floor"));
  Serial.println(F("  p     : print current status"));
  Serial.println(F("  h     : print this help"));
}

void setAngleEstimate(int estimate, const __FlashStringHelper *label) {
  angle = constrain(estimate, SERVO_HARD_MIN, SERVO_HARD_MAX);
  Serial.print(F("Angle estimate set to "));
  Serial.print(label);
  Serial.println(F(" with no movement."));
  printStatus();
}

bool saveCurrentAsFloor() {
  if (angle >= cal.topAngle - SERVO_CALIB_MIN_SPAN) {
    Serial.println(F("Floor rejected: needs safe distance below top."));
    return false;
  }
  cal.floorAngle = angle;
  cal.magic = CALIB_MAGIC;
  saveCalibration();
  Serial.println(F("Saved current angle as floor."));
  printStatus();
  return true;
}

bool saveCurrentAsTop() {
  if (angle <= cal.floorAngle + SERVO_CALIB_MIN_SPAN) {
    Serial.println(F("Top rejected: needs safe distance above floor."));
    return false;
  }
  cal.topAngle = angle;
  cal.magic = CALIB_MAGIC;
  saveCalibration();
  Serial.println(F("Saved current angle as top."));
  printStatus();
  return true;
}

void resetCalibrationToDefaults() {
  resetCalibration();
  Serial.println(F("Calibration reset to defaults."));
  printStatus();
}

void homeHereAndReset() {
  if (!saveCurrentAsFloor()) return;
  p = {0, 0, 0, 0, 0, 0};
  save();
  homed = true;
  updateBLE();
  Serial.println(F("Homing complete. Progress reset at floor."));
  printStatus();
}

void handleSerialCommand(char c) {
  if (c == 'b') {
    setAngleEstimate(cal.floorAngle, F("floor"));
  } else if (c == 'm') {
    setAngleEstimate((cal.floorAngle + cal.topAngle) / 2, F("midpoint"));
  } else if (c == 'u') {
    setAngleEstimate(cal.topAngle, F("top"));
  } else if (c == '+' || c == '-') {
    moveServoRaw(angle + (c == '+' ? 1 : -1));
    printStatus();
  } else if (c == '>' || c == '<') {
    moveServoRaw(angle + (c == '>' ? 5 : -5));
    printStatus();
  } else if (c == '0') {
    moveServoRaw(cal.floorAngle);
    printStatus();
  } else if (c == '9') {
    moveServoRaw(cal.topAngle);
    printStatus();
  } else if (c == 'f') {
    saveCurrentAsFloor();
  } else if (c == 't') {
    saveCurrentAsTop();
  } else if (c == 'H') {
    homeHereAndReset();
  } else if (c == 'd') {
    resetCalibrationToDefaults();
  } else if (c == 'r') {
    if (!homed) {
      Serial.println(F("Reset blocked: home first with H."));
    } else {
      resetProgress();
      printStatus();
    }
  } else if (c == 'p') {
    printStatus();
  } else if (c == 'h') {
    printCalibrationHelp();
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  load();
  printCalibrationHelp();
  if (START_IN_HOMING_MODE) {
    homed = false;
    angle = cal.floorAngle;
    Serial.println(F("Manual homing mode active. No startup movement."));
    Serial.println(F("Pick an estimate with b/m/u, nudge to the floor, then send H."));
  } else {
    homed = true;
    angle = constrain(targetAngle(), SERVO_HARD_MIN, SERVO_HARD_MAX);
    moveServoRaw(angle);
  }
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
      if (!homed) {
        Serial.println(F("BLE command ignored: device not homed."));
      } else if (cmd == CMD_LOG_ACTIVITY) logActivity();
      else if (cmd == CMD_RESET) resetProgress();
    }
  } else {
    connectionSoundPlayed = false;
  }
  BLE.poll();
}
