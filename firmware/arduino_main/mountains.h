/**
 * mountains.h — Mountain definitions for Peak Progress
 *
 * Defines the Mountain struct, LED color palettes, melody types,
 * and the static library of mountains across 4 tiers.
 *
 * LED palettes, tier info, and melody types are guarded behind
 * HAS_CIRCUIT_PLAYGROUND to save flash and SRAM when the CP is absent.
 *
 * Part of the Peak Progress tangible interface project.
 * M2 IHM TII — Tangible Interfaces 2026
 */

#ifndef MOUNTAINS_H
#define MOUNTAINS_H

#include <Arduino.h>

// ─── Compile Flags ──────────────────────────────────────────
// Circuit Playground board is connected via serial (D0/D1)
#define HAS_CIRCUIT_PLAYGROUND

// ─── Constants ──────────────────────────────────────────────
// Full 9-mountain library active with Circuit Playground
#define NUM_MOUNTAINS 9
#define NUM_LEDS      10  // FUTURE: used with CP LED palettes

// ─── Circuit Playground extras (melody types + LED palettes) ─
#ifdef HAS_CIRCUIT_PLAYGROUND

// FUTURE: restored with CP board
enum MelodyType : uint8_t {
  MELODY_SIMPLE,
  MELODY_MODERATE,
  MELODY_TRIUMPHANT,
  MELODY_EPIC,
  MELODY_LEGENDARY
};

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
// Core fields always present.
// FUTURE: weeksRequired, tier, ledColors, melody re-added with CP board.
struct Mountain {
  const char*      name;
  uint8_t          totalSessions;
  uint8_t          unlockAfterSummits;
#ifdef HAS_CIRCUIT_PLAYGROUND
  // FUTURE: restored with CP board
  uint8_t          weeksRequired;
  uint8_t          tier;
  const uint32_t*  ledColors;
  MelodyType       melody;
#endif
};

// ─── Mountain Library ───────────────────────────────────────
// Without CP: 3-mountain testing set (saves flash + SRAM).
// FUTURE: restore all 9 mountains when Circuit Playground is connected.
const Mountain MOUNTAIN_LIBRARY[NUM_MOUNTAINS] = {
#ifdef HAS_CIRCUIT_PLAYGROUND
  // Full library — restored with CP board (NUM_MOUNTAINS must be 9)
  {"Colline Locale",       7, 0, 1, 1, PALETTE_BLUE_GREEN,          MELODY_SIMPLE     },
  {"Petit Sommet",         7, 1, 1, 1, PALETTE_BLUE_CYAN_GREEN,     MELODY_SIMPLE     },
  {"Mont d'Entrainement",  7, 2, 1, 1, PALETTE_PURPLE_GOLD,         MELODY_MODERATE   },
  {"Mont Blanc",          14, 3, 2, 2, PALETTE_ICE_BLUE_WHITE_GOLD, MELODY_TRIUMPHANT },
  {"Matterhorn",          14, 4, 2, 2, PALETTE_DARK_PURPLE_RED,     MELODY_TRIUMPHANT },
  {"Kilimanjaro",         21, 5, 3, 3, PALETTE_WARM_GRADIENT,       MELODY_EPIC       },
  {"Denali",              21, 6, 3, 3, PALETTE_ARCTIC_SHIMMER,      MELODY_EPIC       },
  {"Everest",             28, 7, 4, 4, PALETTE_RAINBOW_SHIMMER,     MELODY_LEGENDARY  },
  {"K2",                  28, 8, 4, 4, PALETTE_FIRE_PULSE,          MELODY_LEGENDARY  },
#else
  // Testing set — 3 tier-1 mountains, no CP needed
  {"Colline Locale",       7, 0},
  {"Petit Sommet",         7, 1},
  {"Mont d'Entrainement",  7, 2},
  /* FUTURE: restore remaining 6 mountains with CP board (set NUM_MOUNTAINS 9):
  {"Mont Blanc",          14, 3},
  {"Matterhorn",          14, 4},
  {"Kilimanjaro",         21, 5},
  {"Denali",              21, 6},
  {"Everest",             28, 7},
  {"K2",                  28, 8},
  */
#endif
};

// ─── Helper ─────────────────────────────────────────────────
// FUTURE: restored with CP board
#ifdef HAS_CIRCUIT_PLAYGROUND
inline uint32_t getMountainColor(const Mountain &mtn, uint8_t ledIndex) {
  if (ledIndex >= NUM_LEDS) return 0;
  return pgm_read_dword(&mtn.ledColors[ledIndex]);
}
#endif

#endif // MOUNTAINS_H
