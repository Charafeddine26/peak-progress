/**
 * circuit_playground.ino — Circuit Playground for Peak Progress v2.0
 *
 * Adafruit Circuit Playground Express
 * Acts as I/O peripheral connected to Arduino Uno WiFi Rev.2
 * via Serial1 (hardware UART on TX/RX pins).
 *
 * Handles:
 *  - Left button press → sends 'T' to Arduino
 *  - NeoPixel LED progress display (L/C commands)
 *  - Melodies (M command, 5 types)
 *  - Single tones (T command)
 *  - Weekly milestone animation (W command)
 *  - Summit celebration animations (S command, tier 1-4)
 *
 * Wiring:
 *   CP TX (#1) → Arduino D4
 *   CP RX (#0) → Arduino D5
 *   CP GND → Arduino GND
 *
 * Baud rate: 9600, newline-terminated messages.
 *
 * M2 IHM TII — Tangible Interfaces 2026
 */

#include <Adafruit_CircuitPlayground.h>

// ─── Configuration ──────────────────────────────────────────
#define DEBOUNCE_MS      1500
#define SERIAL_BAUD      9600
#define LED_BRIGHTNESS   30
#define NUM_PIXELS       10

// ─── State ──────────────────────────────────────────────────
unsigned long lastTouchTime = 0;
uint32_t currentColors[NUM_PIXELS] = {0};

// ─── Notes ──────────────────────────────────────────────────
#define NOTE_C4  262
#define NOTE_G4  392
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_C6  1047
#define NOTE_E6  1319

// ═══════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
  CircuitPlayground.begin();
  Serial.begin(SERIAL_BAUD);    // USB (debug)
  Serial1.begin(SERIAL_BAUD);   // TX/RX pins (to Arduino)
  CircuitPlayground.setBrightness(LED_BRIGHTNESS);
  CircuitPlayground.clearPixels();

  // Boot confirmation: brief green flash
  for (int i = 0; i < NUM_PIXELS; i++) {
    CircuitPlayground.setPixelColor(i, 0, 50, 0);
  }
  delay(300);
  CircuitPlayground.clearPixels();
}

// ═══════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════
void loop() {
  handleButton();
  processSerial();
}

// ═══════════════════════════════════════════════════════════
//  BUTTON DETECTION
// ═══════════════════════════════════════════════════════════
void handleButton() {
  if (CircuitPlayground.leftButton()) {
    unsigned long now = millis();
    if (now - lastTouchTime >= DEBOUNCE_MS) {
      lastTouchTime = now;
      Serial1.println('T');

      // Brief white flash feedback
      for (int i = 0; i < NUM_PIXELS; i++) {
        CircuitPlayground.setPixelColor(i, 80, 80, 80);
      }
      delay(100);
      restoreColors();
    }
  }
}

// ═══════════════════════════════════════════════════════════
//  SERIAL COMMAND PROCESSING
// ═══════════════════════════════════════════════════════════
void processSerial() {
  if (!Serial1.available()) return;

  char cmd = Serial1.read();
  switch (cmd) {
    case 'L': parseLED(); break;
    case 'C': clearLEDs(); break;
    case 'M': parseMelody(); break;
    case 'T': parseTone(); break;
    case 'W': weeklyAnimation(); break;
    case 'S': parseSummit(); break;
    default: break;
  }
}

// ═══════════════════════════════════════════════════════════
//  COMMAND PARSERS
// ═══════════════════════════════════════════════════════════

// L<idx>:<RRGGBB>\n
void parseLED() {
  int idx = Serial1.parseInt();
  Serial1.read(); // skip ':'
  String hex = Serial1.readStringUntil('\n');
  long color = strtol(hex.c_str(), NULL, 16);

  if (idx >= 0 && idx < NUM_PIXELS) {
    currentColors[idx] = (uint32_t)color;
    CircuitPlayground.setPixelColor(idx,
      (color >> 16) & 0xFF,
      (color >> 8) & 0xFF,
      color & 0xFF);
  }
}

// C\n
void clearLEDs() {
  while (Serial1.available() && Serial1.peek() != '\n') Serial1.read();
  if (Serial1.available()) Serial1.read();

  CircuitPlayground.clearPixels();
  for (int i = 0; i < NUM_PIXELS; i++) currentColors[i] = 0;
}

// M<type>\n — melody (0=simple, 1=moderate, 2=triumphant, 3=epic, 4=legendary)
void parseMelody() {
  int type = Serial1.parseInt();
  Serial1.readStringUntil('\n');

  switch (type) {
    case 0: melodySimple(); break;
    case 1: melodyModerate(); break;
    case 2: melodyTriumphant(); break;
    case 3: melodyEpic(); break;
    case 4: melodyLegendary(); break;
    default: melodySimple(); break;
  }
}

// T<freq>:<dur>\n — single tone
void parseTone() {
  int freq = Serial1.parseInt();
  Serial1.read(); // skip ':'
  int dur = Serial1.parseInt();
  Serial1.readStringUntil('\n');

  if (freq > 0 && dur > 0) {
    CircuitPlayground.playTone(freq, dur);
  }
}

// S<tier>\n — summit celebration
void parseSummit() {
  int tier = Serial1.parseInt();
  Serial1.readStringUntil('\n');
  summitAnimation(constrain(tier, 1, 4));
}

// ═══════════════════════════════════════════════════════════
//  MELODIES
// ═══════════════════════════════════════════════════════════

void melodySimple() {
  CircuitPlayground.playTone(NOTE_C5, 200); delay(250);
  CircuitPlayground.playTone(NOTE_E5, 200); delay(250);
  CircuitPlayground.playTone(NOTE_G5, 400); delay(450);
}

void melodyModerate() {
  CircuitPlayground.playTone(NOTE_C5, 150); delay(200);
  CircuitPlayground.playTone(NOTE_E5, 150); delay(200);
  CircuitPlayground.playTone(NOTE_G5, 150); delay(200);
  CircuitPlayground.playTone(NOTE_C6, 400); delay(450);
}

void melodyTriumphant() {
  CircuitPlayground.playTone(NOTE_C5, 150); delay(200);
  CircuitPlayground.playTone(NOTE_E5, 150); delay(200);
  CircuitPlayground.playTone(NOTE_G5, 150); delay(200);
  CircuitPlayground.playTone(NOTE_C6, 200); delay(250);
  CircuitPlayground.playTone(NOTE_E6, 500); delay(550);
}

void melodyEpic() {
  CircuitPlayground.playTone(NOTE_G4, 200); delay(250);
  CircuitPlayground.playTone(NOTE_C5, 200); delay(250);
  CircuitPlayground.playTone(NOTE_E5, 200); delay(250);
  CircuitPlayground.playTone(NOTE_G5, 200); delay(250);
  CircuitPlayground.playTone(NOTE_C6, 300); delay(350);
  CircuitPlayground.playTone(NOTE_E6, 600); delay(650);
}

void melodyLegendary() {
  int notes[] = {NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6, NOTE_E6,
                 NOTE_G5, NOTE_C6, NOTE_E6};
  int durs[]  = {120, 120, 120, 200, 150, 120, 200, 700};
  for (int i = 0; i < 8; i++) {
    CircuitPlayground.playTone(notes[i], durs[i]);
    delay(durs[i] + 30);
  }
}

// ═══════════════════════════════════════════════════════════
//  ANIMATIONS
// ═══════════════════════════════════════════════════════════

// Weekly milestone: gentle white pulse (3 cycles)
void weeklyAnimation() {
  while (Serial1.available() && Serial1.peek() != '\n') Serial1.read();
  if (Serial1.available()) Serial1.read();

  for (int cycle = 0; cycle < 3; cycle++) {
    for (int b = 0; b <= 100; b += 5) {
      for (int i = 0; i < NUM_PIXELS; i++)
        CircuitPlayground.setPixelColor(i, b, b, b);
      delay(10);
    }
    for (int b = 100; b >= 0; b -= 5) {
      for (int i = 0; i < NUM_PIXELS; i++)
        CircuitPlayground.setPixelColor(i, b, b, b);
      delay(10);
    }
  }
  restoreColors();
}

// Summit celebration — tier-specific animation + melody
void summitAnimation(int tier) {
  switch (tier) {
    case 1: melodySimple();     summitTier1(); break;
    case 2: melodyModerate();   summitTier2(); break;
    case 3: melodyTriumphant(); summitTier3(); break;
    case 4: melodyLegendary();  summitTier4(); break;
    default: melodySimple();    summitTier1(); break;
  }
  restoreColors();
}

// Tier 1: Green pulse
void summitTier1() {
  for (int cycle = 0; cycle < 3; cycle++) {
    for (int b = 0; b <= 150; b += 10) {
      for (int i = 0; i < NUM_PIXELS; i++)
        CircuitPlayground.setPixelColor(i, 0, b, 0);
      delay(15);
    }
    for (int b = 150; b >= 0; b -= 10) {
      for (int i = 0; i < NUM_PIXELS; i++)
        CircuitPlayground.setPixelColor(i, 0, b, 0);
      delay(15);
    }
  }
}

// Tier 2: Blue/white shimmer
void summitTier2() {
  for (int cycle = 0; cycle < 3; cycle++) {
    for (int i = 0; i < NUM_PIXELS; i++) {
      CircuitPlayground.setPixelColor(i, 100, 100, 255);
      delay(80);
    }
    delay(200);
    for (int i = NUM_PIXELS - 1; i >= 0; i--) {
      CircuitPlayground.setPixelColor(i, 255, 255, 255);
      delay(80);
    }
    delay(200);
    CircuitPlayground.clearPixels();
    delay(100);
  }
}

// Tier 3: Gold cascade
void summitTier3() {
  for (int cycle = 0; cycle < 4; cycle++) {
    for (int i = 0; i < NUM_PIXELS; i++) {
      CircuitPlayground.setPixelColor(i, 255, 200, 0);
      delay(60);
    }
    delay(100);
    for (int i = 0; i < NUM_PIXELS; i++) {
      CircuitPlayground.setPixelColor(i, 255, 100, 0);
      delay(40);
    }
    delay(100);
    CircuitPlayground.clearPixels();
    delay(80);
  }
}

// Tier 4: Rainbow cascade
void summitTier4() {
  for (int cycle = 0; cycle < 5; cycle++) {
    for (int offset = 0; offset < 256; offset += 8) {
      for (int i = 0; i < NUM_PIXELS; i++) {
        uint8_t hue = (i * 25 + offset) & 0xFF;
        uint32_t c = colorWheel(hue);
        CircuitPlayground.setPixelColor(i,
          (c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
      }
      delay(20);
    }
  }
}

// ═══════════════════════════════════════════════════════════
//  UTILITY
// ═══════════════════════════════════════════════════════════
void restoreColors() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    CircuitPlayground.setPixelColor(i,
      (currentColors[i] >> 16) & 0xFF,
      (currentColors[i] >> 8) & 0xFF,
      currentColors[i] & 0xFF);
  }
}

uint32_t colorWheel(byte pos) {
  if (pos < 85) {
    return ((uint32_t)(pos * 3) << 16) |
           ((uint32_t)(255 - pos * 3) << 8);
  } else if (pos < 170) {
    pos -= 85;
    return ((uint32_t)(255 - pos * 3) << 16) |
           ((uint32_t)(pos * 3));
  } else {
    pos -= 170;
    return ((uint32_t)(pos * 3) << 8) |
           (255 - pos * 3);
  }
}
