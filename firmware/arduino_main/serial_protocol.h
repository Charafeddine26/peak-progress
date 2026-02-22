/**
 * serial_protocol.h — Arduino ↔ Circuit Playground serial communication
 *
 * Defines the text-based serial protocol for sending LED commands,
 * animation triggers, and tone requests from the Arduino to the
 * Circuit Playground, and receiving touch events back.
 *
 * Baud rate: 9600
 * All messages are newline-terminated (\n).
 *
 * ── Arduino → Circuit Playground Commands ──
 *   L<idx>:<RRGGBB>   Set single LED to hex color
 *   C                  Clear all LEDs
 *   R<start>:<end>:<RRGGBB>  Set LED range to color
 *   M<type>            Play melody (0=simple, 1=moderate, 2=triumphant, 3=epic, 4=legendary)
 *   T<freq>:<dur>      Play tone at frequency for duration ms
 *   W                  Trigger weekly milestone animation
 *   S<tier>            Trigger summit celebration animation (tier 1-4)
 *   P<idx>:<sessions>  Load palette for mountain index, light <sessions> LEDs
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
#include "mountains.h"
#include "progress.h"

// ─── Outgoing Commands (Arduino → CP) ──────────────────────

/**
 * Send a command to set a single LED on the Circuit Playground.
 */
inline void sendLEDCommand(uint8_t ledIndex, uint32_t color) {
  Serial.print('L');
  Serial.print(ledIndex);
  Serial.print(':');
  // Send as 6-char hex with leading zeros
  if (color < 0x100000) Serial.print('0');
  if (color < 0x010000) Serial.print('0');
  if (color < 0x001000) Serial.print('0');
  if (color < 0x000100) Serial.print('0');
  if (color < 0x000010) Serial.print('0');
  Serial.println(color, HEX);
}

/**
 * Send a command to clear all LEDs.
 */
inline void sendClearLEDs() {
  Serial.println('C');
}

/**
 * Send the current mountain's palette and light up LEDs based on progress.
 * Lights LEDs 0 through (ledsToLight - 1) with the mountain's palette colors.
 */
inline void sendProgressLEDs(const UserProgress &prog, const Mountain &mtn) {
  uint8_t totalSess = SESSIONS_FOR_MOUNTAIN(mtn);
  int ledsToLight = 0;

  if (totalSess > 0) {
    ledsToLight = ((int)prog.sessionsOnCurrentMountain * NUM_LEDS) / totalSess;
    ledsToLight = constrain(ledsToLight, 0, NUM_LEDS);
  }

  // Turn off all first
  sendClearLEDs();
  delay(50);

  // Light up progress LEDs
  for (int i = 0; i < ledsToLight; i++) {
    uint32_t color = getMountainColor(mtn, i);
    sendLEDCommand(i, color);
    delay(20); // Small delay to avoid serial buffer overflow
  }
}

/**
 * Send a melody play command.
 */
inline void sendMelodyCommand(MelodyType melody) {
  Serial.print('M');
  Serial.println((int)melody);
}

/**
 * Send a single tone command.
 */
inline void sendToneCommand(uint16_t frequency, uint16_t durationMs) {
  Serial.print('T');
  Serial.print(frequency);
  Serial.print(':');
  Serial.println(durationMs);
}

/**
 * Send weekly milestone animation trigger.
 */
inline void sendWeeklyAnimation() {
  Serial.println('W');
}

/**
 * Send summit celebration animation trigger.
 * @param tier  Mountain tier (1-4) for tier-specific animation
 */
inline void sendSummitAnimation(uint8_t tier) {
  Serial.print('S');
  Serial.println(tier);
}

// ─── Incoming Events (CP → Arduino) ────────────────────────

/**
 * Check if a touch event was received from the Circuit Playground.
 * Call in the main loop. Returns true if 'T' was received.
 */
inline bool serialTouchDetected() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'T') {
      // Flush any remaining characters (newline etc.)
      while (Serial.available()) Serial.read();
      return true;
    }
  }
  return false;
}

#endif // SERIAL_PROTOCOL_H
