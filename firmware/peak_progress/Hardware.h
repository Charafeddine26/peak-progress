#ifndef HARDWARE_H
#define HARDWARE_H
#include "Config.h"
#include "Memory.h"

int calibratedFloorAngle() { return cal.floorAngle; }
int calibratedTopAngle() { return cal.topAngle; }

// ─── Buzzer ─────────────────────────────────────────────────
void playNote(int frequency, int duration) {
  int volumePercent = 2;
  long period = 1000000L / frequency;
  long pulseWidth = (period * volumePercent) / 100;
  long offTime = period - pulseWidth;
  
  unsigned long startTime = millis();
  while (millis() - startTime < duration) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(pulseWidth);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(offTime);
  }
}

void playConnect() {
  int notes[] = {600, 1000};
  for (uint8_t i = 0; i < (sizeof(notes) / sizeof(notes[0])); i++) {
    playNote(notes[i], 150);
    delay(40);
  }
  noTone(BUZZER_PIN);
}

void playLog(){
  playNote(800, 200);
  noTone(BUZZER_PIN);
}

void playSummit(){
  int notes[] = {1000, 800, 600};
  playNote(1200,180);
  delay(40);
  playNote(1200,140);
  delay(40);
  playNote(1200,180);
  delay(40);
  for (uint8_t i = 0; i < (sizeof(notes) / sizeof(notes[0])); i++) {
    playNote(notes[i], 100);
    delay(40);
  }
  playNote(1200,400);
  noTone(BUZZER_PIN);
}


// ─── Servo Control (direct pulse, no library) ───────────────
void servoPulse(int deg) {
  int us = 544 + ((long)deg * (2400 - 544)) / 180;
  digitalWrite(SERVO_PIN, HIGH);
  delayMicroseconds(us);
  digitalWrite(SERVO_PIN, LOW);
}

void moveServoWithin(int target, int minAngle, int maxAngle) {
  target = constrain(target, minAngle, maxAngle);
  pinMode(SERVO_PIN, OUTPUT);
  int step = (target > angle) ? 1 : -1;
  while (angle != target) {
    angle += step;
    servoPulse(angle);
    delay(STEP_DELAY);
  }
  // Hold position briefly then stop pulsing
  for (uint8_t i = 0; i < 25; i++) {
    servoPulse(angle);
    delay(20);
  }
  digitalWrite(SERVO_PIN, LOW);
}

void moveServo(int target) {
  int lo = min(calibratedFloorAngle(), calibratedTopAngle());
  int hi = max(calibratedFloorAngle(), calibratedTopAngle());
  moveServoWithin(target, lo, hi);
}

void moveServoRaw(int target) {
  moveServoWithin(target, SERVO_HARD_MIN, SERVO_HARD_MAX);
}

int targetAngle() {
  uint8_t total = SESSIONS_FOR(p.mtn);
  return calibratedFloorAngle() +
         ((long)p.sessions * (calibratedTopAngle() - calibratedFloorAngle())) / total;
}

void logActivity() {
  uint8_t total = SESSIONS_FOR(p.mtn);
  if (p.sessions >= total) return;
  playLog();
  
  p.sessions++;
  p.total++;
  p.streak++;
  if (p.streak > p.bestStreak) p.bestStreak = p.streak;

  moveServo(targetAngle());
  save();
  updateBLE();  // Send completed state (e.g. 7/7) to app immediately

  if (p.sessions >= total) {
    delay(3000);  // Let app show the summit state
    p.summits++;
    p.mtn = nextMtn();
    p.sessions = 0;
    save();
    playSummit();
    moveServo(calibratedFloorAngle());
    updateBLE();  // Send new mountain state (0/next) to app
  }
}

void resetProgress() {
  p = {0, 0, 0, 0, 0, 0};
  save();
  moveServo(calibratedFloorAngle());
  updateBLE();
}

#endif
