/**
 * serial_protocol.h — Arduino ↔ Circuit Playground serial communication
 *
 * Uses SoftwareSerial on D2 (RX) / D3 (TX) to avoid conflict with
 * the USB serial on D0/D1. This means:
 *   - No need to disconnect wires when uploading firmware
 *   - USB Serial Monitor still works for debugging
 *
 * Wiring:
 *   Arduino D2 → Circuit Playground TX
 *   Arduino D3 → Circuit Playground RX
 *   Arduino GND → Circuit Playground GND
 *
 * Baud rate: 9600
 * All messages are newline-terminated (\n).
 *
 * ── Arduino → Circuit Playground Commands ──
 *   L<idx>:<RRGGBB>   Set single LED to hex color
 *   C                  Clear all LEDs
 *   M<type>            Play melody (0-4)
 *   T<freq>:<dur>      Play tone at frequency for duration ms
 *   W                  Trigger weekly milestone animation
 *   S<tier>            Trigger summit celebration animation (tier 1-4)
 *
 * ── Circuit Playground → Arduino Events ──
 *   T                  Touch detected (user tapped capacitive pad)
 *
 * Part of the Peak Progress tangible interface project.
 * M2 IHM TII — Tangible Interfaces 2026
 */

#ifndef SERIAL_PROTOCOL_H
#define SERIAL_PROTOCOL_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "mountains.h"
#include "progress.h"

// D2 = RX (receives from CP TX), D3 = TX (sends to CP RX)
#define CP_RX_PIN 2
#define CP_TX_PIN 3

static SoftwareSerial cpSerial(CP_RX_PIN, CP_TX_PIN);

/**
 * Initialize the SoftwareSerial connection to Circuit Playground.
 */
inline void initCPSerial() {
  cpSerial.begin(9600);
}

// ─── Outgoing Commands (Arduino → CP) ──────────────────────

inline void sendLEDCommand(uint8_t ledIndex, uint32_t color) {
  cpSerial.print('L');
  cpSerial.print(ledIndex);
  cpSerial.print(':');
  if (color < 0x100000) cpSerial.print('0');
  if (color < 0x010000) cpSerial.print('0');
  if (color < 0x001000) cpSerial.print('0');
  if (color < 0x000100) cpSerial.print('0');
  if (color < 0x000010) cpSerial.print('0');
  cpSerial.println(color, HEX);
}

inline void sendClearLEDs() {
  cpSerial.println('C');
}

inline void sendProgressLEDs(const UserProgress &prog, const Mountain &mtn) {
  uint8_t totalSess = SESSIONS_FOR_MOUNTAIN(mtn);
  int ledsToLight = 0;

  if (totalSess > 0) {
    ledsToLight = ((int)prog.sessionsOnCurrentMountain * NUM_LEDS) / totalSess;
    ledsToLight = constrain(ledsToLight, 0, NUM_LEDS);
  }

  sendClearLEDs();
  delay(50);

  for (int i = 0; i < ledsToLight; i++) {
    uint32_t color = getMountainColor(mtn, i);
    sendLEDCommand(i, color);
    delay(20);
  }
}

inline void sendMelodyCommand(MelodyType melody) {
  cpSerial.print('M');
  cpSerial.println((int)melody);
}

inline void sendToneCommand(uint16_t frequency, uint16_t durationMs) {
  cpSerial.print('T');
  cpSerial.print(frequency);
  cpSerial.print(':');
  cpSerial.println(durationMs);
}

inline void sendWeeklyAnimation() {
  cpSerial.println('W');
}

inline void sendSummitAnimation(uint8_t tier) {
  cpSerial.print('S');
  cpSerial.println(tier);
}

// ─── Incoming Events (CP → Arduino) ────────────────────────

inline bool serialTouchDetected() {
  if (cpSerial.available()) {
    char c = cpSerial.read();
    if (c == 'T') {
      while (cpSerial.available()) cpSerial.read();
      return true;
    }
  }
  return false;
}

#endif // SERIAL_PROTOCOL_H
