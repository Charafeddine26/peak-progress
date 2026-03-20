#ifndef HARDWARE_H
#define HARDWARE_H
#include "Config.h"
#include "Memory.h"


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
  int notes[] = {600, 1000,};
  for (int i = 0; i < 3; i++) {
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
  for (int i = 0; i < 4; i++) {
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

void moveServo(int target) {
  target = constrain(target, SERVO_MIN, SERVO_MAX);
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

int targetAngle() {
  return SERVO_MIN + ((long)p.sessions * (SERVO_MAX - SERVO_MIN)) / SESSIONS_PER;
}

void logActivity() {
  if (p.sessions >= SESSIONS_PER) return;
  playLog();
  
  p.sessions++;
  p.total++;
  p.streak++;
  if (p.streak > p.bestStreak) p.bestStreak = p.streak;

  moveServo(targetAngle());
  save();

  if (p.sessions >= SESSIONS_PER) {
    delay(3000);
    p.summits++;
    p.mtn = nextMtn();
    p.sessions = 0;
    save();
    playSummit();
    moveServo(SERVO_MIN);
  }

  updateBLE();
}

void resetProgress() {
  p = {0, 0, 0, 0, 0, 0};
  save();
  moveServo(SERVO_MIN);
  updateBLE();
}

#endif