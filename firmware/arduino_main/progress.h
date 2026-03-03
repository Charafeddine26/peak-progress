/**
 * progress.h — User progress tracking and EEPROM persistence
 *
 * Manages the UserProgress struct, EEPROM save/load/reset,
 * and mountain unlock logic via a simple bitfield.
 *
 * EEPROM Memory Map:
 *   Address 0       : Magic byte (0xA5) — detects first-time init
 *   Address 1-12    : UserProgress struct (12 bytes)
 *   Address 13-1023 : Reserved
 *
 * Part of the Peak Progress tangible interface project.
 * M2 IHM TII — Tangible Interfaces 2026
 */

#ifndef PROGRESS_H
#define PROGRESS_H

#include <EEPROM.h>
#include "mountains.h"

// ─── EEPROM Layout ──────────────────────────────────────────
#define EEPROM_MAGIC       0xA5
#define EEPROM_ADDR_MAGIC  0
#define EEPROM_ADDR_DATA   1

// ─── Fast Test Mode ─────────────────────────────────────────
// Uncomment to reduce sessions-per-mountain to 3 for rapid testing
// #define FAST_TEST_MODE

#ifdef FAST_TEST_MODE
  #define SESSIONS_FOR_MOUNTAIN(mtn) 3
#else
  #define SESSIONS_FOR_MOUNTAIN(mtn) ((mtn).totalSessions)
#endif

// ─── User Progress Structure ────────────────────────────────
struct UserProgress {
  uint16_t totalSessionsAllTime;       // Career total (0-65535)
  uint8_t  currentMountainIndex;       // Index into MOUNTAIN_LIBRARY (0-8)
  uint8_t  sessionsOnCurrentMountain;  // Sessions completed on current climb
  uint8_t  summitsReached;             // Total mountains summited
  uint16_t unlockedBitfield;           // Bit i = mountain i unlocked
  uint8_t  currentStreakDays;          // Consecutive active days
  uint8_t  longestStreakDays;          // Best streak ever
  uint8_t  _reserved[2];              // Padding for future use
};
// Total size: 12 bytes

// ─── EEPROM Functions ───────────────────────────────────────

/**
 * Initialize progress to defaults (first mountain unlocked, all else zero).
 */
inline void initProgressDefaults(UserProgress &prog) {
  memset(&prog, 0, sizeof(UserProgress));
  prog.currentMountainIndex = 0;
  prog.unlockedBitfield = 0x0001; // Bit 0 = Colline Locale unlocked
}

/**
 * Save progress to EEPROM. Call only on state changes (not in loop).
 */
inline void saveProgress(const UserProgress &prog) {
  EEPROM.put(EEPROM_ADDR_DATA, prog);
}

/**
 * Load progress from EEPROM. If magic byte is missing, initializes defaults.
 */
inline void loadProgress(UserProgress &prog) {
  uint8_t magic = EEPROM.read(EEPROM_ADDR_MAGIC);

  if (magic != EEPROM_MAGIC) {
    // First boot — initialize
    initProgressDefaults(prog);
    EEPROM.write(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
    saveProgress(prog);
    return;
  }

  EEPROM.get(EEPROM_ADDR_DATA, prog);
}

/**
 * Full reset: clear progress and re-initialize.
 */
inline void resetProgress(UserProgress &prog) {
  initProgressDefaults(prog);
  saveProgress(prog);
}

// ─── Mountain Unlock Logic ──────────────────────────────────

/**
 * Check if a specific mountain is unlocked.
 */
inline bool isMountainUnlocked(const UserProgress &prog, uint8_t mountainIndex) {
  if (mountainIndex >= NUM_MOUNTAINS) return false;
  return (prog.unlockedBitfield >> mountainIndex) & 0x01;
}

/**
 * Unlock a specific mountain by index.
 */
inline void unlockMountain(UserProgress &prog, uint8_t mountainIndex) {
  if (mountainIndex >= NUM_MOUNTAINS) return;
  prog.unlockedBitfield |= (1 << mountainIndex);
}

/**
 * Check all mountains and unlock any that the user has earned.
 * Called after each summit completion.
 *
 * Logic: Mountain i is unlocked if summitsReached >= MOUNTAIN_LIBRARY[i].unlockAfterSummits
 */
inline void checkAndUnlockMountains(UserProgress &prog) {
  for (uint8_t i = 0; i < NUM_MOUNTAINS; i++) {
    if (prog.summitsReached >= MOUNTAIN_LIBRARY[i].unlockAfterSummits) {
      unlockMountain(prog, i);
    }
  }
}

/**
 * Find the next unlocked mountain after the current one.
 * Returns the index, or current index if nothing new is available.
 */
inline uint8_t findNextMountain(const UserProgress &prog) {
  uint8_t next = prog.currentMountainIndex + 1;
  if (next < NUM_MOUNTAINS && isMountainUnlocked(prog, next)) {
    return next;
  }
  // All summited or next locked — wrap to challenge mode (re-climb)
  return 0;
}

#endif // PROGRESS_H
