#ifndef MEMORY_H
#define MEMORY_H
#include <EEPROM.h>
#include "Config.h"

bool calibrationIsValid(const Calibration &value) {
  return value.magic == CALIB_MAGIC &&
         value.floorAngle >= SERVO_HARD_MIN &&
         value.topAngle <= SERVO_HARD_MAX &&
         value.floorAngle < value.topAngle &&
         (value.topAngle - value.floorAngle) >= SERVO_CALIB_MIN_SPAN;
}

void saveCalibration() { EEPROM.put(EEPROM_CALIB_ADDR, cal); }

void resetCalibration() {
  cal = {CALIB_MAGIC, SERVO_DEFAULT_MIN, SERVO_DEFAULT_MAX};
  saveCalibration();
}

void loadCalibration() {
  Calibration stored;
  EEPROM.get(EEPROM_CALIB_ADDR, stored);
  if (calibrationIsValid(stored)) {
    cal = stored;
  } else {
    resetCalibration();
  }
}

void load() {
  if (EEPROM.read(0) != EEPROM_MAGIC) {
    p = {0, 0, 0, 0, 0, 0};
    EEPROM.write(0, EEPROM_MAGIC);
    EEPROM.put(EEPROM_PROGRESS_ADDR, p);
    resetCalibration();
    return;
  }
  EEPROM.get(EEPROM_PROGRESS_ADDR, p);
  if (p.mtn >= NUM_MTNS) {
    p = {0, 0, 0, 0, 0, 0};
    EEPROM.put(EEPROM_PROGRESS_ADDR, p);
  }
  loadCalibration();
}

// ─── Core Logic ──────────────────────────────────────────────

uint8_t nextMtn() {
  uint8_t next = p.mtn + 1;
  if (next < NUM_MTNS && p.summits >= next) return next;
  return 0;
}
void save() { EEPROM.put(EEPROM_PROGRESS_ADDR, p); }
#endif
