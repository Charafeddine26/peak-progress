/*
 * BLE Characteristics:
 *   Progress (0x0001) — 8 bytes, read/notify
 *   Command  (0x0003) — 1 byte, write (0x01=log, 0x02=reset)
*/
#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H
#include <ArduinoBLE.h>
#include "Config.h"

// ─── BLE Init ──────────────────────────────────────────────
void setBLE() {
  BLE.setLocalName("PeakProgress");
  BLE.setAdvertisedService(peakService);
  peakService.addCharacteristic(progressChar);
  peakService.addCharacteristic(commandChar);
  BLE.addService(peakService);
  BLE.advertise();
}


// ─── BLE Update ──────────────────────────────────────────────
void updateBLE() {
  uint8_t data[8] = {
    p.mtn, p.sessions, SESSIONS_FOR(p.mtn),
    p.summits, p.streak, p.bestStreak,
    (uint8_t)(p.total >> 8), (uint8_t)(p.total & 0xFF)
  };
  progressChar.writeValue(data, 8);
}
#endif