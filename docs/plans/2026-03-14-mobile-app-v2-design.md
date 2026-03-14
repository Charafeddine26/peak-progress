# Mobile App v2.0 — Design Document

## Context

The firmware was rebuilt from scratch on branch `2.0` with only 2 BLE characteristics:
- **Progress** (0x0001) — Read/Notify, 8 bytes
- **Command** (0x0003) — Write, 1 byte (0x01=log, 0x02=reset)

The old app crashes immediately because it tries to fetch Mountain (0x0002) and Unlock (0x0004) characteristics that no longer exist.

## Goal

Rebuild `app.js` from scratch to work with the v2.0 firmware. Keep `index.html` and `style.css` as-is — the UI structure and styling are fine.

## Data Model

### 8-byte Progress Payload (from firmware)

```
Byte 0: mountain index (0-2)
Byte 1: sessions on current mountain (0-3)
Byte 2: sessions needed per mountain (3)
Byte 3: summits reached
Byte 4: current streak
Byte 5: best streak
Byte 6-7: total sessions (uint16, big-endian)
```

### Derived Data (computed in JS, not from BLE)

- **Mountain name**: `MOUNTAINS[index].name` — hardcoded array matching firmware
- **Mountain unlock**: mountain `i` is unlocked if `summits >= i` — same logic as firmware
- **Progress percentage**: `sessions / sessionsNeeded * 100`
- **Tier**: all 3 current mountains are tier 1

### Mountain Library (JS-side, must match firmware)

```javascript
const MOUNTAINS = [
  { name: 'Colline Locale',      sessions: 3, tier: 1, unlockAfter: 0 },
  { name: 'Petit Sommet',        sessions: 3, tier: 1, unlockAfter: 1 },
  { name: 'Mont Entrainement',   sessions: 3, tier: 1, unlockAfter: 2 },
];
```

Note: firmware uses `SESSIONS_PER = 3` (fast test mode). The app reads byte 2 from the progress payload so it auto-adapts if firmware changes this value.

## Architecture

### BLE Layer

```
connectBLE()
  -> requestDevice({ filters: [{ services: [SERVICE_UUID] }] })
  -> gatt.connect()
  -> getPrimaryService()
  -> getCharacteristic(PROGRESS_UUID)  // only this one
  -> getCharacteristic(COMMAND_UUID)   // only this one
  -> startNotifications on Progress
  -> initial readValue on Progress
  -> update UI

onProgressChanged(event)
  -> parse 8 bytes into state object
  -> derive mountain name, unlock bitfield from state
  -> updateAllUI()

sendCommand(cmd)
  -> writeValue(new Uint8Array([cmd])) to Command characteristic

onDisconnected()
  -> update status indicators
  -> show toast
```

### State Object

```javascript
let state = {
  mountainIndex: 0,
  sessionsOnMountain: 0,
  sessionsNeeded: 3,
  summits: 0,
  streak: 0,
  longestStreak: 0,
  totalSessions: 0,
};
```

No `mountainName` or `unlockedBitfield` in state — these are derived on the fly from `mountainIndex` and `summits`.

### Demo Mode

Demo mode simulates the same state object. When user taps "Log Activity" in demo:
1. Increment sessions, total, streak
2. If sessions >= mountain's session count: increment summits, advance mountain, reset sessions
3. Update UI

Same logic as firmware, same state shape. No inconsistency.

### UI Updates

All 5 screens update from the single state object:

1. **Home**: mountain name from `MOUNTAINS[state.mountainIndex]`, progress bar, stats grid
2. **Mountains**: loop `MOUNTAINS`, check `state.summits >= mtn.unlockAfter` for unlock status
3. **History**: stats from state, timeline from mountainIndex, achievements from conditions
4. **Settings**: connection status, device name, mode
5. **Connection**: status dot and text

### Error Handling

- `connectBLE()`: try/catch around entire flow, specific error messages
- `getCharacteristic()`: if either fails, show which one and stay on connection screen
- `writeValue()`: try/catch, show toast on failure
- Disconnect: update UI, don't crash
- Button cooldown: 1 second after log/reset to prevent double-tap

## What Changes

| File | Action | Details |
|------|--------|---------|
| `app.js` | **Rewrite from scratch** | New BLE logic (2 chars), derived data, consistent demo mode |
| `index.html` | **Minor edits** | Update BLE Protocol version to v2.0, remove emoji references from mountain icons |
| `style.css` | **No changes** | Keep as-is |

## What Gets Removed

- `CHAR_MOUNTAIN_UUID` and `CHAR_UNLOCK_UUID` constants
- `charMountain` and `charUnlock` variables
- `parseMountainName()` and `parseUnlock()` functions
- `onMountainChanged()` and `onUnlockChanged()` handlers
- `state.mountainName` and `state.unlockedBitfield` fields
- `LED_PALETTES` array (no LED visualization needed without CP)
- Mountain `gradient` field (simplify to tier-based colors)

## Implementation Plan

### Task 1: Write new app.js

Single file, clean structure:
1. Constants (2 UUIDs, 2 commands, mountain data, tier data, achievements)
2. State (single object, no derived fields stored)
3. BLE connection (2 characteristics only)
4. BLE notification handler (parse 8 bytes, derive, update UI)
5. Demo mode (simulates same state)
6. User actions (log, reset, confirm modal)
7. UI updaters (dashboard, mountain list, history, settings)
8. Navigation and toast
9. Init

### Task 2: Update index.html

- Change BLE Protocol version from v1.0 to v2.0
- Mountain item icons: use tier number instead of emoji codes (01, 02, etc.)
- Achievement icons: same — use simple text or symbols

### Task 3: Test

- Open in Chrome, verify demo mode works end-to-end
- Verify all 5 screens render correctly
- Verify log activity progresses climber
- Verify summit triggers mountain change
- Verify reset works
- Verify mountain list shows correct lock/unlock status
