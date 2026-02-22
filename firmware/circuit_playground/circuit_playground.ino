/**
 * circuit_playground.ino — Circuit Playground firmware for Peak Progress
 *
 * Adafruit Circuit Playground Classic/Express
 * Handles: capacitive touch detection, NeoPixel LED control,
 * buzzer audio feedback, and serial communication with Arduino.
 *
 * This board acts as the I/O interface:
 *  - Detects user taps (capacitive touch pad #3)
 *  - Receives LED and audio commands from Arduino via serial
 *  - Executes LED animations (weekly, summit celebrations)
 *  - Plays melodies via onboard buzzer
 *
 * Serial Protocol (see serial_protocol.h on Arduino side):
 *   Incoming: L<idx>:<hex>, C, M<type>, T<freq>:<dur>, W, S<tier>
 *   Outgoing: T (touch detected)
 *
 * Baud rate: 9600
 *
 * Part of the Peak Progress tangible interface project.
 * M2 IHM TII — Tangible Interfaces 2026
 */

#include <Adafruit_CircuitPlayground.h>

// ─── Configuration ──────────────────────────────────────────
#define TOUCH_PAD        3       // Capacitive touch pad number
#define TOUCH_THRESHOLD  500     // Capacitance threshold for detection
#define DEBOUNCE_MS      1500    // Prevent accidental double-taps (1.5s)
#define SERIAL_BAUD      9600
#define LED_BRIGHTNESS   30      // NeoPixel brightness (0-255), low for comfort
#define NUM_PIXELS       10

// ─── State ──────────────────────────────────────────────────
unsigned long lastTouchTime = 0;
uint32_t currentColors[NUM_PIXELS] = {0}; // Current LED color state

// ─── Melody Definitions ────────────────────────────────────
// Note frequencies (Hz)
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_C6  1047
#define NOTE_E6  1319

// ═══════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════
void setup() {
  CircuitPlayground.begin();
  Serial.begin(SERIAL_BAUD);

  // Set LED brightness
  CircuitPlayground.setBrightness(LED_BRIGHTNESS);

  // Clear all LEDs
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
  // 1. Check for capacitive touch
  handleTouch();

  // 2. Process serial commands from Arduino
  processSerialCommands();
}

// ═══════════════════════════════════════════════════════════
//  TOUCH DETECTION
// ═══════════════════════════════════════════════════════════

/**
 * Read capacitive touch pad with debounce.
 * Sends 'T' over serial when a valid tap is detected.
 */
void handleTouch() {
  uint16_t capValue = CircuitPlayground.readCap(TOUCH_PAD);

  if (capValue > TOUCH_THRESHOLD) {
    unsigned long now = millis();
    if (now - lastTouchTime >= DEBOUNCE_MS) {
      lastTouchTime = now;

      // Send touch event to Arduino
      Serial.println('T');

      // Visual feedback: brief white flash on all LEDs
      for (int i = 0; i < NUM_PIXELS; i++) {
        CircuitPlayground.setPixelColor(i, 80, 80, 80);
      }
      delay(100);

      // Restore current colors
      restoreColors();
    }
  }
}

// ═══════════════════════════════════════════════════════════
//  SERIAL COMMAND PROCESSING
// ═══════════════════════════════════════════════════════════

/**
 * Parse and execute serial commands from the Arduino.
 * Protocol is text-based, newline-terminated.
 */
void processSerialCommands() {
  if (!Serial.available()) return;

  char cmd = Serial.read();

  switch (cmd) {
    case 'L': parseLEDCommand(); break;    // Set single LED
    case 'C': clearAllLEDs(); break;       // Clear all
    case 'M': parseMelodyCommand(); break; // Play melody
    case 'T': parseToneCommand(); break;   // Play tone (T<freq>:<dur>)
    case 'W': weeklyAnimation(); break;    // Weekly milestone
    case 'S': parseSummitCommand(); break; // Summit celebration
    default:
      // Skip unknown characters (including newlines)
      break;
  }
}

/**
 * Parse: L<idx>:<RRGGBB>\n
 * Example: L3:00FF99
 */
void parseLEDCommand() {
  int ledIndex = Serial.parseInt();
  Serial.read(); // Skip ':'
  long colorVal = Serial.parseInt(); // Read hex color

  // Sometimes parseInt doesn't handle hex well, try readString
  // For robustness, read until newline
  String hexStr = Serial.readStringUntil('\n');

  // Parse hex manually if needed
  if (colorVal == 0 && hexStr.length() > 0) {
    colorVal = strtol(hexStr.c_str(), NULL, 16);
  }

  if (ledIndex >= 0 && ledIndex < NUM_PIXELS) {
    currentColors[ledIndex] = (uint32_t)colorVal;
    uint8_t r = (colorVal >> 16) & 0xFF;
    uint8_t g = (colorVal >> 8) & 0xFF;
    uint8_t b = colorVal & 0xFF;
    CircuitPlayground.setPixelColor(ledIndex, r, g, b);
  }
}

/**
 * Parse: C\n — Clear all LEDs
 */
void clearAllLEDs() {
  // Read until newline
  while (Serial.available() && Serial.peek() != '\n') Serial.read();
  if (Serial.available()) Serial.read(); // consume newline

  CircuitPlayground.clearPixels();
  for (int i = 0; i < NUM_PIXELS; i++) {
    currentColors[i] = 0;
  }
}

/**
 * Parse: M<type>\n — Play melody
 * Types: 0=simple, 1=moderate, 2=triumphant, 3=epic, 4=legendary
 */
void parseMelodyCommand() {
  int melodyType = Serial.parseInt();
  // Consume rest of line
  Serial.readStringUntil('\n');

  switch (melodyType) {
    case 0: playSimpleMelody(); break;
    case 1: playModerateMelody(); break;
    case 2: playTriumphantMelody(); break;
    case 3: playEpicMelody(); break;
    case 4: playLegendaryMelody(); break;
    default: playSimpleMelody(); break;
  }
}

/**
 * Parse: T<freq>:<dur>\n — Play single tone
 * Note: 'T' is also the touch event BUT only Circuit Playground sends 'T',
 * Arduino sends 'T' as tone command. Context differentiates them.
 */
void parseToneCommand() {
  // T is already consumed. Read freq:dur
  int freq = Serial.parseInt();
  Serial.read(); // Skip ':'
  int dur = Serial.parseInt();
  Serial.readStringUntil('\n');

  if (freq > 0 && dur > 0) {
    CircuitPlayground.playTone(freq, dur);
  }
}

/**
 * Parse: S<tier>\n — Summit celebration
 */
void parseSummitCommand() {
  int tier = Serial.parseInt();
  Serial.readStringUntil('\n');
  summitAnimation(constrain(tier, 1, 4));
}

// ═══════════════════════════════════════════════════════════
//  MELODY FUNCTIONS
// ═══════════════════════════════════════════════════════════

// Tier 1: Simple 3-note melody
void playSimpleMelody() {
  CircuitPlayground.playTone(NOTE_C5, 200);
  delay(250);
  CircuitPlayground.playTone(NOTE_E5, 200);
  delay(250);
  CircuitPlayground.playTone(NOTE_G5, 400);
  delay(450);
}

// Tier 1 advanced: Moderate 4-note melody
void playModerateMelody() {
  CircuitPlayground.playTone(NOTE_C5, 150);
  delay(200);
  CircuitPlayground.playTone(NOTE_E5, 150);
  delay(200);
  CircuitPlayground.playTone(NOTE_G5, 150);
  delay(200);
  CircuitPlayground.playTone(NOTE_C6, 400);
  delay(450);
}

// Tier 2: Triumphant 5-note melody
void playTriumphantMelody() {
  CircuitPlayground.playTone(NOTE_C5, 150);
  delay(200);
  CircuitPlayground.playTone(NOTE_E5, 150);
  delay(200);
  CircuitPlayground.playTone(NOTE_G5, 150);
  delay(200);
  CircuitPlayground.playTone(NOTE_C6, 200);
  delay(250);
  CircuitPlayground.playTone(NOTE_E6, 500);
  delay(550);
}

// Tier 3: Epic 6-note melody
void playEpicMelody() {
  CircuitPlayground.playTone(NOTE_G4, 200);
  delay(250);
  CircuitPlayground.playTone(NOTE_C5, 200);
  delay(250);
  CircuitPlayground.playTone(NOTE_E5, 200);
  delay(250);
  CircuitPlayground.playTone(NOTE_G5, 200);
  delay(250);
  CircuitPlayground.playTone(NOTE_C6, 300);
  delay(350);
  CircuitPlayground.playTone(NOTE_E6, 600);
  delay(650);
}

// Tier 4: Legendary 8-note fanfare
void playLegendaryMelody() {
  int notes[] = {NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6, NOTE_E6,
                 NOTE_G5, NOTE_C6, NOTE_E6};
  int durs[]  = {120, 120, 120, 200, 150, 120, 200, 700};

  for (int i = 0; i < 8; i++) {
    CircuitPlayground.playTone(notes[i], durs[i]);
    delay(durs[i] + 30);
  }
}

// ═══════════════════════════════════════════════════════════
//  LED ANIMATIONS
// ═══════════════════════════════════════════════════════════

/**
 * Weekly milestone animation: gentle white pulse (3 cycles).
 */
void weeklyAnimation() {
  // Consume rest of line
  while (Serial.available() && Serial.peek() != '\n') Serial.read();
  if (Serial.available()) Serial.read();

  for (int cycle = 0; cycle < 3; cycle++) {
    // Fade up
    for (int brightness = 0; brightness <= 100; brightness += 5) {
      for (int i = 0; i < NUM_PIXELS; i++) {
        CircuitPlayground.setPixelColor(i, brightness, brightness, brightness);
      }
      delay(10);
    }
    // Fade down
    for (int brightness = 100; brightness >= 0; brightness -= 5) {
      for (int i = 0; i < NUM_PIXELS; i++) {
        CircuitPlayground.setPixelColor(i, brightness, brightness, brightness);
      }
      delay(10);
    }
  }

  // Restore saved colors
  restoreColors();
}

/**
 * Summit celebration animation — tier-specific.
 * Tier 1: Green pulse
 * Tier 2: Blue/white shimmer
 * Tier 3: Gold cascade
 * Tier 4: Full rainbow cascade
 */
void summitAnimation(int tier) {
  switch (tier) {
    case 1: summitTier1(); break;
    case 2: summitTier2(); break;
    case 3: summitTier3(); break;
    case 4: summitTier4(); break;
    default: summitTier1(); break;
  }

  // Restore saved colors after animation
  restoreColors();
}

// Tier 1: Green pulse (3 cycles)
void summitTier1() {
  for (int cycle = 0; cycle < 3; cycle++) {
    for (int b = 0; b <= 150; b += 10) {
      for (int i = 0; i < NUM_PIXELS; i++) {
        CircuitPlayground.setPixelColor(i, 0, b, 0);
      }
      delay(15);
    }
    for (int b = 150; b >= 0; b -= 10) {
      for (int i = 0; i < NUM_PIXELS; i++) {
        CircuitPlayground.setPixelColor(i, 0, b, 0);
      }
      delay(15);
    }
  }
}

// Tier 2: Blue/white shimmer (sequential lighting)
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

// Tier 4: Full rainbow cascade (the ultimate celebration)
void summitTier4() {
  for (int cycle = 0; cycle < 5; cycle++) {
    for (int offset = 0; offset < 256; offset += 8) {
      for (int i = 0; i < NUM_PIXELS; i++) {
        uint8_t hue = (i * 25 + offset) & 0xFF;
        uint32_t color = colorWheel(hue);
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        CircuitPlayground.setPixelColor(i, r, g, b);
      }
      delay(20);
    }
  }
}

// ═══════════════════════════════════════════════════════════
//  UTILITY
// ═══════════════════════════════════════════════════════════

/**
 * Restore LEDs to their saved color state (after animations).
 */
void restoreColors() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    uint8_t r = (currentColors[i] >> 16) & 0xFF;
    uint8_t g = (currentColors[i] >> 8) & 0xFF;
    uint8_t b = currentColors[i] & 0xFF;
    CircuitPlayground.setPixelColor(i, r, g, b);
  }
}

/**
 * Color wheel function for rainbow effects.
 * Input: 0-255, output: RGB color cycling through rainbow.
 */
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
