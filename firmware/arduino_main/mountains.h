/**
 * mountains.h — Mountain definitions for Peak Progress
 *
 * Defines the Mountain struct, LED color palettes, melody types,
 * and the static library of 9 mountains across 4 tiers.
 *
 * LED palettes are guarded behind HAS_CIRCUIT_PLAYGROUND to save
 * flash when the Circuit Playground is not connected.
 *
 * Part of the Peak Progress tangible interface project.
 * M2 IHM TII — Tangible Interfaces 2026
 */

#ifndef MOUNTAINS_H
#define MOUNTAINS_H

#include <Arduino.h>

// ─── Compile Flags ──────────────────────────────────────────
// Uncomment when Circuit Playground is connected:
#define HAS_CIRCUIT_PLAYGROUND

// ─── Constants ──────────────────────────────────────────────
#define NUM_MOUNTAINS 9
#define NUM_LEDS      10

// ─── Melody Types ───────────────────────────────────────────
enum MelodyType : uint8_t {
  MELODY_SIMPLE,
  MELODY_MODERATE,
  MELODY_TRIUMPHANT,
  MELODY_EPIC,
  MELODY_LEGENDARY
};

// ─── LED Color Palettes (only when CP is connected) ─────────
#ifdef HAS_CIRCUIT_PLAYGROUND

const uint32_t PALETTE_BLUE_GREEN[NUM_LEDS] PROGMEM = {
  0x0000FF, 0x0022DD, 0x0044BB, 0x006699, 0x008877,
  0x00AA55, 0x00CC33, 0x00EE11, 0x22FF00, 0x44FF00
};
const uint32_t PALETTE_BLUE_CYAN_GREEN[NUM_LEDS] PROGMEM = {
  0x0000FF, 0x0033FF, 0x0066FF, 0x0099FF, 0x00CCFF,
  0x00FFCC, 0x00FF99, 0x00FF66, 0x00FF33, 0x00FF00
};
const uint32_t PALETTE_PURPLE_GOLD[NUM_LEDS] PROGMEM = {
  0x6600CC, 0x7711BB, 0x8822AA, 0x993399, 0xAA4488,
  0xBB5577, 0xCC7744, 0xDD9922, 0xEEBB11, 0xFFDD00
};
const uint32_t PALETTE_ICE_BLUE_WHITE_GOLD[NUM_LEDS] PROGMEM = {
  0x00FFFF, 0x33FFFF, 0x66FFFF, 0x99FFFF, 0xCCFFFF,
  0xFFFFFF, 0xFFFFCC, 0xFFFF99, 0xFFDD44, 0xFFAA00
};
const uint32_t PALETTE_DARK_PURPLE_RED[NUM_LEDS] PROGMEM = {
  0x000066, 0x110077, 0x220088, 0x440099, 0x6600AA,
  0x8800BB, 0xAA0099, 0xCC0066, 0xDD0033, 0xFF0000
};
const uint32_t PALETTE_WARM_GRADIENT[NUM_LEDS] PROGMEM = {
  0xFF6600, 0xFF7711, 0xFF8822, 0xFF9933, 0xFFAA44,
  0xFFBB55, 0xFFCC44, 0xFFDD33, 0xFFAA22, 0xFF4400
};
const uint32_t PALETTE_ARCTIC_SHIMMER[NUM_LEDS] PROGMEM = {
  0xFFFFFF, 0xDDEEFF, 0xBBDDFF, 0x99CCFF, 0x77BBFF,
  0x55AAFF, 0x3399FF, 0x1188FF, 0x0077FF, 0x0044FF
};
const uint32_t PALETTE_RAINBOW_SHIMMER[NUM_LEDS] PROGMEM = {
  0xFF0000, 0xFF7F00, 0xFFFF00, 0x00FF00, 0x0000FF,
  0x4B0082, 0x9400D3, 0xFF00FF, 0xFF66FF, 0xFFFFFF
};
const uint32_t PALETTE_FIRE_PULSE[NUM_LEDS] PROGMEM = {
  0x330000, 0x660000, 0x990000, 0xCC0000, 0xFF0000,
  0xFF3300, 0xFF6600, 0xFF9900, 0xFFCC00, 0xFFFF00
};

#endif // HAS_CIRCUIT_PLAYGROUND

// ─── Mountain Definition ────────────────────────────────────
struct Mountain {
  const char*      name;
  uint8_t          weeksRequired;
  uint8_t          totalSessions;
  uint8_t          tier;
#ifdef HAS_CIRCUIT_PLAYGROUND
  const uint32_t*  ledColors;
#endif
  MelodyType       melody;
  uint8_t          unlockAfterSummits;
};

// ─── Mountain Library ───────────────────────────────────────
const Mountain MOUNTAIN_LIBRARY[NUM_MOUNTAINS] = {
#ifdef HAS_CIRCUIT_PLAYGROUND
  {"Colline Locale",       1,  7, 1, PALETTE_BLUE_GREEN,          MELODY_SIMPLE,     0},
  {"Petit Sommet",         1,  7, 1, PALETTE_BLUE_CYAN_GREEN,     MELODY_SIMPLE,     1},
  {"Mont d'Entrainement",  1,  7, 1, PALETTE_PURPLE_GOLD,         MELODY_MODERATE,   2},
  {"Mont Blanc",           2, 14, 2, PALETTE_ICE_BLUE_WHITE_GOLD, MELODY_TRIUMPHANT, 3},
  {"Matterhorn",           2, 14, 2, PALETTE_DARK_PURPLE_RED,     MELODY_TRIUMPHANT, 4},
  {"Kilimanjaro",          3, 21, 3, PALETTE_WARM_GRADIENT,       MELODY_EPIC,       5},
  {"Denali",               3, 21, 3, PALETTE_ARCTIC_SHIMMER,      MELODY_EPIC,       6},
  {"Everest",              4, 28, 4, PALETTE_RAINBOW_SHIMMER,     MELODY_LEGENDARY,  7},
  {"K2",                   4, 28, 4, PALETTE_FIRE_PULSE,          MELODY_LEGENDARY,  8}
#else
  {"Colline Locale",       1,  7, 1, MELODY_SIMPLE,     0},
  {"Petit Sommet",         1,  7, 1, MELODY_SIMPLE,     1},
  {"Mont d'Entrainement",  1,  7, 1, MELODY_MODERATE,   2},
  {"Mont Blanc",           2, 14, 2, MELODY_TRIUMPHANT, 3},
  {"Matterhorn",           2, 14, 2, MELODY_TRIUMPHANT, 4},
  {"Kilimanjaro",          3, 21, 3, MELODY_EPIC,       5},
  {"Denali",               3, 21, 3, MELODY_EPIC,       6},
  {"Everest",              4, 28, 4, MELODY_LEGENDARY,  7},
  {"K2",                   4, 28, 4, MELODY_LEGENDARY,  8}
#endif
};

// ─── Helper ─────────────────────────────────────────────────
#ifdef HAS_CIRCUIT_PLAYGROUND
inline uint32_t getMountainColor(const Mountain &mtn, uint8_t ledIndex) {
  if (ledIndex >= NUM_LEDS) return 0;
  return pgm_read_dword(&mtn.ledColors[ledIndex]);
}
#endif

#endif // MOUNTAINS_H
