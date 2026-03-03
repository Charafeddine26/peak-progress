/**
 * arduino_main.ino — Main firmware for Peak Progress
 *
 * Arduino Uno WiFi Rev.2 — Main controller
 * Orchestrates: servo, BLE, EEPROM persistence, mountain progression,
 * and LCD display. Serial to Circuit Playground is disabled when
 * HAS_CIRCUIT_PLAYGROUND is not defined in mountains.h.
 *
 * Part of the Peak Progress tangible interface project.
 * M2 IHM TII — Tangible Interfaces 2026
 */

#include "mountains.h"
#include "progress.h"
#include "servo_control.h"
#include "ble_service.h"
#include "display.h"

#ifdef HAS_CIRCUIT_PLAYGROUND
#include "serial_protocol.h"
#endif

// ─── State ──────────────────────────────────────────────────
UserProgress userProgress;

// ═══════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
#ifdef HAS_CIRCUIT_PLAYGROUND
  Serial.begin(9600);
#endif
  delay(500);

  // Load progress from EEPROM
  loadProgress(userProgress);
  checkAndUnlockMountains(userProgress);

  // Initialize servo
  initServo();
  positionClimberForCurrentProgress(userProgress);

  // Initialize BLE
  if (initBLE()) {
    updateAllBLE(userProgress);
  }

  // Initialize LCD display
  initDisplay();
  updateDisplay(userProgress);

#ifdef HAS_CIRCUIT_PLAYGROUND
  delay(500);
  const Mountain &mtn = MOUNTAIN_LIBRARY[userProgress.currentMountainIndex];
  sendProgressLEDs(userProgress, mtn);
#endif
}

// ═══════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════
void loop() {
#ifdef HAS_CIRCUIT_PLAYGROUND
  if (serialTouchDetected()) {
    logActivity();
  }
#endif

  uint8_t bleCmd = checkBLECommand();
  if (bleCmd != 0x00) {
    handleBLECommand(bleCmd);
  }
}

// ═══════════════════════════════════════════════════════════
//  LOG ACTIVITY
// ═══════════════════════════════════════════════════════════
void logActivity() {
  const Mountain &mtn = MOUNTAIN_LIBRARY[userProgress.currentMountainIndex];
  uint8_t totalSess = SESSIONS_FOR_MOUNTAIN(mtn);

  if (userProgress.sessionsOnCurrentMountain >= totalSess) return;

  userProgress.sessionsOnCurrentMountain++;
  userProgress.totalSessionsAllTime++;
  userProgress.currentStreakDays++;

  if (userProgress.currentStreakDays > userProgress.longestStreakDays) {
    userProgress.longestStreakDays = userProgress.currentStreakDays;
  }

  // Move climber
  int angle = calculateServoAngle(
    userProgress.sessionsOnCurrentMountain, totalSess
  );
  moveClimberSmooth(angle);

#ifdef HAS_CIRCUIT_PLAYGROUND
  sendProgressLEDs(userProgress, mtn);
  sendToneCommand(440 + (userProgress.sessionsOnCurrentMountain * 40), 150);
#endif

  // Save to EEPROM
  saveProgress(userProgress);

  // Check summit
  if (userProgress.sessionsOnCurrentMountain >= totalSess) {
    summitReached();
  }

  updateAllBLE(userProgress);
  updateDisplay(userProgress);
}

// ═══════════════════════════════════════════════════════════
//  SUMMIT REACHED
// ═══════════════════════════════════════════════════════════
void summitReached() {
  const Mountain &mtn = MOUNTAIN_LIBRARY[userProgress.currentMountainIndex];

#ifdef HAS_CIRCUIT_PLAYGROUND
  sendSummitAnimation(mtn.tier);
  sendMelodyCommand(mtn.melody);
#endif

  displaySummitReached(mtn.name);
  delay(3000);

  userProgress.summitsReached++;
  checkAndUnlockMountains(userProgress);

  uint8_t nextIdx = findNextMountain(userProgress);

  if (nextIdx != userProgress.currentMountainIndex) {
    displayNewMountain(MOUNTAIN_LIBRARY[nextIdx].name);
    delay(2000);
  }

  userProgress.currentMountainIndex = nextIdx;
  userProgress.sessionsOnCurrentMountain = 0;

  saveProgress(userProgress);
  descendToBase();

#ifdef HAS_CIRCUIT_PLAYGROUND
  sendClearLEDs();
#endif

  updateDisplay(userProgress);
  updateAllBLE(userProgress);
}

// ═══════════════════════════════════════════════════════════
//  BLE COMMAND HANDLER
// ═══════════════════════════════════════════════════════════
void handleBLECommand(uint8_t cmd) {
  switch (cmd) {
    case CMD_LOG_ACTIVITY:
      logActivity();
      break;

    case CMD_RESET:
      resetProgress(userProgress);
      descendToBase();
#ifdef HAS_CIRCUIT_PLAYGROUND
      sendClearLEDs();
#endif
      updateAllBLE(userProgress);
      updateDisplay(userProgress);
      break;

    default:
      break;
  }
}
