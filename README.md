# Peak Progress

A tangible mountain-climbing interface. Log physical activity by touching a sensor at the base of a 3D-printed mountain — a climber figure ascends as progress accumulates across a nine-mountain challenge.

---

## Current Hardware Status

| Component                   | Status          | Role                                  |
| --------------------------- | --------------- | ------------------------------------- |
| Arduino Uno WiFi Rev.2      | ✅ Everyone has | BLE host, servo control, EEPROM       |
| Adafruit Circuit Playground | 🔜 Next week   | Touch sensor, NeoPixels, buzzer       |
| SG90 Micro Servo            | ✅ From kit     | Drives the climber up the mountain    |
| Grove OLED 0.96" (optional) | ✅ From kit     | Local display                         |

---

## Repository Structure

```
firmware/
  arduino_main/       ← Upload to Arduino Uno WiFi Rev.2
  circuit_playground/ ← Upload to Adafruit Circuit Playground (next week)
mobile-app/
  index.html          ← Web companion app
  app.js
  style.css
  key.pem / cert.pem  ← Self-signed SSL certs (already included)
docs/                 ← Full protocol and wiring documentation
```

---

## Quick Start

### Step 1 — Clone

```bash
git clone https://github.com/swae2/peak-progress.git
cd peak-progress
```

### Step 2 — Install Arduino Library

In Arduino IDE: **Sketch → Include Library → Manage Libraries**, install:
- `ArduinoBLE`

### Step 3 — Upload Firmware to Arduino

1. Plug Arduino Uno WiFi Rev.2 into USB
2. Arduino IDE: **Tools → Board → Arduino Uno WiFi Rev2**
3. Arduino IDE: **Tools → Port → (select the Arduino port)**
4. Open `firmware/arduino_main/arduino_main.ino`
5. Click **Upload**

### Step 4 — Enable Fast Test Mode

In `firmware/arduino_main/progress.h`, uncomment line 23:

```cpp
#define FAST_TEST_MODE
```

This reduces each mountain from 7–28 sessions down to **3**, so you can test the full 9-mountain progression quickly. Re-upload after the change.

### Step 5 — Run the Web App

```bash
cd mobile-app
npx http-server ./ -S -C cert.pem -K key.pem -p 8443
```

Open **https://localhost:8443** in Chrome. Accept the self-signed cert warning.

> **Browser support:** Chrome on Android and desktop (Windows/macOS/Linux). iOS is not supported — see `docs/ios_ble_limitation.md`.

### Step 6 — Connect and Test

1. Click **Connect to Device** in the web app
2. Select **PeakProgress** from the Bluetooth popup
3. Use the **Log Activity** button in the app to simulate a tap → servo should move, LEDs light up
4. Check **Serial Monitor** (9600 baud) for debug output

> The physical touch input (Circuit Playground pad #3) is not available yet. For now, use the web app's Log Activity button to drive the device.

---

## Alternative: Build with arduino-cli

If your sketch exceeds 100% flash when compiling in the Arduino IDE, it's likely due to mismatched board core or library versions. The `build.bat` script compiles with **pinned versions** to guarantee a consistent binary size.

### One-time setup

1. Install arduino-cli:
   ```
   winget install ArduinoSA.CLI
   ```
2. Restart your terminal

### Build

```bash
cd firmware
build.bat
```

The script automatically downloads the correct core (`arduino:megaavr@1.8.8`) and library (`ArduinoBLE@1.5.0`) on first run. Subsequent runs reuse the cached versions.

> This does **not** replace the Arduino IDE — you still upload firmware from the IDE. This just verifies your build compiles with the correct versions.

---

## BLE Reference

| Characteristic | UUID suffix | Properties    | Content                             |
| -------------- | ----------- | ------------- | ----------------------------------- |
| Progress       | `...0001`   | Read + Notify | 8 bytes: sessions, streaks, totals  |
| Mountain       | `...0002`   | Read + Notify | 20-byte string: mountain name       |
| Command        | `...0003`   | Write         | `0x01` = log activity, `0x02` = reset |
| Unlock         | `...0004`   | Read + Notify | 2-byte bitfield: unlocked mountains |

Service UUID: `19b10000-e8f2-537e-4f6c-d104768a1214`

Full spec: `docs/BLE_protocol.md`

---

## Docs

| File                              | Contents                              |
| --------------------------------- | ------------------------------------- |
| `docs/BLE_protocol.md`            | Full BLE characteristic spec          |
| `docs/wiring_schematic.md`        | Pin assignments and wiring diagram    |
| `docs/testing_guide_no_3d.md`     | Full test sequence (needs both boards)|
| `docs/EEPROM_memory_map.md`       | Persistent storage layout             |
| `docs/serial_protocol.md`         | Arduino ↔ Circuit Playground serial   |
| `docs/ios_ble_limitation.md`      | Why iOS is not supported + options    |
