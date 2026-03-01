# Peak Progress

A tangible mountain-climbing interface built with Arduino and Web Bluetooth. Users log physical activity by touching a sensor at the base of a 3D-printed mountain — a climber figure ascends the slope as progress accumulates across a nine-mountain challenge.

---

## Hardware

| Component                   | Role                              |
| --------------------------- | --------------------------------- |
| Arduino Uno WiFi Rev.2      | BLE host, servo control, EEPROM   |
| Adafruit Circuit Playground | Touch sensor, NeoPixels, buzzer   |
| SG90 Micro Servo            | Drives the climber up the mountain |
| Grove OLED 0.96" (optional) | Local display                     |

**Key pins:**
- Circuit Playground pad **#3** → capacitive touch (main input)
- Arduino **D8** → servo signal wire (orange)
- Arduino **D0/D1** ↔ Circuit Playground RX/TX → serial link between boards

---

## Repository Structure

```
firmware/
  arduino_main/       ← Upload to Arduino Uno WiFi Rev.2
  circuit_playground/ ← Upload to Adafruit Circuit Playground
mobile-app/
  index.html          ← Web companion app (open in browser)
  app.js
  style.css
  key.pem / cert.pem  ← Self-signed SSL certs (already generated)
docs/                 ← Full protocol and wiring documentation
```

---

## Quick Start

### Step 1 — Clone

```bash
git clone https://github.com/swae2/peak-progress.git
cd peak-progress
```

### Step 2 — Install Arduino Libraries

In Arduino IDE: **Sketch → Include Library → Manage Libraries**, install:
- `ArduinoBLE`
- `Adafruit Circuit Playground`
- `U8g2` (only if using OLED)

### Step 3 — Upload Firmware

> Always **disconnect the D0/D1 serial wires** between boards before uploading, then reconnect after.

**Circuit Playground first:**
1. Plug Circuit Playground into USB
2. Board: `Adafruit Circuit Playground` | Port: (CP port)
3. Open `firmware/circuit_playground/circuit_playground.ino` → Upload

**Then Arduino:**
1. Plug Arduino Uno WiFi Rev.2 into USB
2. Board: `Arduino Uno WiFi Rev2` | Port: (Arduino port)
3. Open `firmware/arduino_main/arduino_main.ino` → Upload

**Reconnect serial wires** (D0↔TX, D1↔RX).

### Step 4 — Enable Fast Test Mode (recommended for testing)

In `firmware/arduino_main/progress.h`, uncomment line 23:

```cpp
#define FAST_TEST_MODE
```

This reduces each mountain from 7–28 sessions down to **3 taps**, so you can run through all 9 mountains in 27 taps instead of 133. Re-upload after changing.

### Step 5 — Test the Hardware

Open **Tools → Serial Monitor** at 9600 baud. Touch pad **#3** on the Circuit Playground and verify:
- Servo rotates
- NeoPixels light up
- Serial monitor shows session count incrementing

See `docs/testing_guide_no_3d.md` for the full test sequence.

### Step 6 — Run the Web App

The web app requires HTTPS (Web Bluetooth restriction). SSL certs are already included.

```bash
cd mobile-app
node -e "
const https = require('https'), fs = require('fs'), path = require('path');
const dir = __dirname;
const srv = https.createServer(
  { key: fs.readFileSync(path.join(dir,'key.pem')), cert: fs.readFileSync(path.join(dir,'cert.pem')) },
  (req, res) => {
    const f = req.url === '/' ? 'index.html' : req.url.slice(1);
    const fp = path.join(dir, f);
    const mime = {'.html':'text/html','.css':'text/css','.js':'application/javascript'};
    res.writeHead(200, {'Content-Type': mime[path.extname(fp)] || 'text/plain'});
    res.end(fs.readFileSync(fp));
  }
);
srv.listen(8443, () => console.log('Open https://localhost:8443'));
"
```

Open **https://localhost:8443** in Chrome. Accept the self-signed cert warning, then click **Connect to Device**.

> **Browser support:** Chrome on Android and desktop (Windows/macOS/Linux). iOS is not supported — see `docs/ios_ble_limitation.md`.

---

## BLE Reference

| Characteristic | UUID suffix | Properties    | Content                        |
| -------------- | ----------- | ------------- | ------------------------------ |
| Progress       | `...0001`   | Read + Notify | 8 bytes: sessions, streaks, totals |
| Mountain       | `...0002`   | Read + Notify | 20-byte string: mountain name  |
| Command        | `...0003`   | Write         | `0x01` = log activity, `0x02` = reset |
| Unlock         | `...0004`   | Read + Notify | 2-byte bitfield: unlocked mountains |

Service UUID: `19b10000-e8f2-537e-4f6c-d104768a1214`

Full spec: `docs/BLE_protocol.md`

---

## Docs

| File                              | Contents                            |
| --------------------------------- | ----------------------------------- |
| `docs/BLE_protocol.md`            | Full BLE characteristic spec        |
| `docs/wiring_schematic.md`        | Pin assignments and wiring diagram  |
| `docs/testing_guide_no_3d.md`     | How to test without the 3D mountain |
| `docs/EEPROM_memory_map.md`       | Persistent storage layout           |
| `docs/serial_protocol.md`         | Arduino ↔ Circuit Playground serial |
| `docs/ios_ble_limitation.md`      | Why iOS is not supported + options  |
