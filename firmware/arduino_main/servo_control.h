/**
 * servo_control.h — Servo motor control for climber figure movement
 *
 * Controls the SG90 micro servo that drives the climber figure
 * up and down the mountain via the pulley system.
 *
 * Servo range is constrained to 20°-170° to avoid endpoint jitter.
 * Movement is animated smoothly via 1° increments with configurable delay.
 *
 * Part of the Peak Progress tangible interface project.
 * M2 IHM TII — Tangible Interfaces 2026
 */

#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Servo.h>
#include "mountains.h"
#include "progress.h"

// ─── Configuration ──────────────────────────────────────────
#define SERVO_PIN        8
#define SERVO_MIN_ANGLE  20    // Base camp position (avoid jitter at 0°)
#define SERVO_MAX_ANGLE  170   // Summit position (avoid jitter at 180°)
#define SERVO_STEP_DELAY 15    // Milliseconds between 1° increments
#define SERVO_SETTLE_MS  500   // Wait after reaching target before detach

// ─── Global Servo Instance ──────────────────────────────────
Servo climberServo;
int currentServoAngle = SERVO_MIN_ANGLE;

// ─── Functions ──────────────────────────────────────────────

/**
 * Initialize servo to base camp position.
 */
inline void initServo() {
  climberServo.attach(SERVO_PIN);
  climberServo.write(SERVO_MIN_ANGLE);
  currentServoAngle = SERVO_MIN_ANGLE;
  delay(SERVO_SETTLE_MS);
}

/**
 * Calculate the servo angle for a given session count and total sessions.
 * Uses long arithmetic to avoid 8-bit overflow.
 *
 * @param sessions  Number of completed sessions (0 to totalSessions)
 * @param totalSessions  Total sessions required for the mountain
 * @return Servo angle in range [SERVO_MIN_ANGLE, SERVO_MAX_ANGLE]
 */
inline int calculateServoAngle(uint8_t sessions, uint8_t totalSessions) {
  if (totalSessions == 0) return SERVO_MIN_ANGLE;
  if (sessions >= totalSessions) return SERVO_MAX_ANGLE;

  int range = SERVO_MAX_ANGLE - SERVO_MIN_ANGLE; // 150
  return SERVO_MIN_ANGLE + (range * (int)sessions) / (int)totalSessions;
}

/**
 * Smoothly animate the servo from current position to target angle.
 * Moves 1° at a time with SERVO_STEP_DELAY between each step.
 *
 * @param targetAngle  Desired servo position (will be constrained)
 */
inline void moveClimberSmooth(int targetAngle) {
  targetAngle = constrain(targetAngle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);

  if (targetAngle == currentServoAngle) return;

  climberServo.attach(SERVO_PIN);  // Re-attach if detached

  int step = (targetAngle > currentServoAngle) ? 1 : -1;

  while (currentServoAngle != targetAngle) {
    currentServoAngle += step;
    climberServo.write(currentServoAngle);
    delay(SERVO_STEP_DELAY);
  }

  delay(SERVO_SETTLE_MS);
  climberServo.detach();  // Reduce jitter and power when idle
}

/**
 * Position the climber based on current progress (used on boot).
 */
inline void positionClimberForCurrentProgress(const UserProgress &prog) {
  const Mountain &mtn = MOUNTAIN_LIBRARY[prog.currentMountainIndex];
  uint8_t totalSess = SESSIONS_FOR_MOUNTAIN(mtn);
  int angle = calculateServoAngle(prog.sessionsOnCurrentMountain, totalSess);
  moveClimberSmooth(angle);
}

/**
 * Smoothly descend climber to base camp (after summit or reset).
 */
inline void descendToBase() {
  moveClimberSmooth(SERVO_MIN_ANGLE);
}

#endif // SERVO_CONTROL_H
