/**
 * ble_service.h — BLE service for Peak Progress
 *
 * Defines BLE service and characteristics for mobile app communication.
 * Uses ArduinoBLE library (Arduino Uno WiFi Rev.2 NINA module).
 *
 * Part of the Peak Progress tangible interface project.
 * M2 IHM TII — Tangible Interfaces 2026
 */

#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <ArduinoBLE.h>
#include "mountains.h"
#include "progress.h"

// ─── UUIDs ──────────────────────────────────────────────────
#define SERVICE_UUID       "19B10000-E8F2-537E-4F6C-D104768A1214"
#define CHAR_PROGRESS_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"
#define CHAR_COMMAND_UUID  "19B10003-E8F2-537E-4F6C-D104768A1214"

// ─── Command Codes ──────────────────────────────────────────
#define CMD_LOG_ACTIVITY   0x01
#define CMD_RESET          0x02
#define CMD_SELECT_MOUNT   0x03

// ─── BLE Objects ────────────────────────────────────────────
BLEService peakService(SERVICE_UUID);
BLECharacteristic progressChar(CHAR_PROGRESS_UUID, BLERead | BLENotify, 8);
BLEByteCharacteristic commandChar(CHAR_COMMAND_UUID, BLEWrite);

inline bool initBLE() {
  if (!BLE.begin()) return false;

  BLE.setLocalName("PeakProgress");
  BLE.setAdvertisedService(peakService);

  peakService.addCharacteristic(progressChar);
  peakService.addCharacteristic(commandChar);

  BLE.addService(peakService);
  BLE.advertise();
  return true;
}

inline void updateBLEProgress(const UserProgress &prog) {
  const Mountain &mtn = MOUNTAIN_LIBRARY[prog.currentMountainIndex];

  uint8_t data[8] = {
    prog.currentMountainIndex,
    prog.sessionsOnCurrentMountain,
    SESSIONS_FOR_MOUNTAIN(mtn),
    prog.summitsReached,
    prog.currentStreakDays,
    prog.longestStreakDays,
    (uint8_t)(prog.totalSessionsAllTime >> 8),
    (uint8_t)(prog.totalSessionsAllTime & 0xFF)
  };

  progressChar.writeValue(data, 8);
}

inline void updateAllBLE(const UserProgress &prog) {
  updateBLEProgress(prog);
}

inline uint8_t checkBLECommand() {
  BLEDevice central = BLE.central();

  if (central && central.connected()) {
    if (commandChar.written()) {
      return commandChar.value();
    }
  }

  BLE.poll();
  return 0x00;
}

#endif // BLE_SERVICE_H
