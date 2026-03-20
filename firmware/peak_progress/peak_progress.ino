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

void setup() {
  Serial.begin(9600);
  load();
  angle = targetAngle();
  moveServo(angle);
  Serial.println("Test");

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
  }
  BLE.poll();
  
}