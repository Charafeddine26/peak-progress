#ifndef MEMORY_H
#define MEMORY_H
#include <EEPROM.h>
#include "Config.h"

void load() {
  if (EEPROM.read(0) != EEPROM_MAGIC) {
    p = {0, 0, 0, 0, 0, 0};
    EEPROM.write(0, EEPROM_MAGIC);
    EEPROM.put(1, p);
    return;
  }
  EEPROM.get(1, p);
  if (p.mtn >= NUM_MTNS) {
    p = {0, 0, 0, 0, 0, 0};
    EEPROM.put(1, p);
  }
}

// ─── Core Logic ──────────────────────────────────────────────

uint8_t nextMtn() {
  uint8_t next = p.mtn + 1;
  if (next < NUM_MTNS && p.summits >= next) return next;
  return 0;
}



void save() { EEPROM.put(1, p); }
#endif