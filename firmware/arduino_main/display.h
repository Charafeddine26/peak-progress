/**
 * display.h — Display stubs (no display hardware connected)
 *
 * All functions are no-ops. When a display is added later,
 * replace this file with the appropriate implementation.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "progress.h"

inline void initDisplay() {}
inline void updateDisplay(const UserProgress &) {}
inline void displaySummitReached(const char*) {}
inline void displayNewMountain(const char*) {}

#endif // DISPLAY_H
