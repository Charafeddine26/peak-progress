/**
 * ble_test.ino — Minimal BLE test sketch
 *
 * Does nothing except start BLE advertising.
 * If "PeakTest" appears in Bluetooth settings, BLE works.
 */

#include <ArduinoBLE.h>

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println("Starting BLE...");

  if (!BLE.begin()) {
    Serial.println("BLE FAILED!");
    while (1); // Stop here
  }

  BLE.setLocalName("PeakTest");
  BLE.advertise();

  Serial.print("BLE OK! Address: ");
  Serial.println(BLE.address());
  Serial.println("Look for 'PeakTest' in Bluetooth settings.");
}

void loop() {
  BLE.poll();
}
