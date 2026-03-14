# Peak Progress

A tangible mountain-climbing interface that gamifies physical activity. Touch a sensor, and a climber figure physically ascends a mountain via a servo-driven pulley. Track your progress from your phone over Bluetooth.

Built for the M2 IHM Tangible Interfaces course, 2026.

---

## Hardware

| Component              | Role                            |
| ---------------------- | ------------------------------- |
| Arduino Uno WiFi Rev.2 | BLE host, servo control, EEPROM |
| SG90 Micro Servo       | Drives the climber figure       |

The servo is wired to **pin 8**. No external libraries needed for servo control — the firmware drives it directly via PWM pulses.

---

## Repository Structure

```
firmware/
  peak_progress/        <- Single-file firmware (upload to Arduino)
  build.bat             <- Compile with pinned versions via arduino-cli
  arduino-cli.yaml      <- arduino-cli configuration
mobile-app/
  index.html            <- Web companion app (5 screens)
  app.js                <- BLE + UI logic
  style.css             <- Dark theme styling
  cert.pem / key.pem    <- Self-signed SSL certs for HTTPS
docs/
  plans/                <- Design documents
```

---

## Quick Start

### 1. Upload Firmware

**Option A — Arduino IDE:**
1. Open `firmware/peak_progress/peak_progress.ino`
2. Install `ArduinoBLE` library (Sketch > Include Library > Manage Libraries)
3. Select board: Arduino Uno WiFi Rev2
4. Upload

**Option B — arduino-cli:**
```bash
cd firmware
build.bat            # compile only
build.bat upload     # compile + upload
```

The build script pins `arduino:megaavr@1.8.8` and `ArduinoBLE@1.5.0` for consistent builds across machines.

### 2. Run the Web App

```bash
cd mobile-app
npx http-server ./ -S -C cert.pem -K key.pem -p 8443
```

Open **https://localhost:8443** in Chrome. Accept the self-signed cert warning.

### 3. Connect

1. Click **Scan & Connect**
2. Select **PeakProgress** from the Bluetooth popup
3. Use **Log Activity** to drive the climber

Or use **Demo Mode** to test the app without hardware.

---

## BLE Protocol

Service UUID: `19b10000-e8f2-537e-4f6c-d104768a1214`

| Characteristic | UUID suffix | Properties    | Content                               |
| -------------- | ----------- | ------------- | ------------------------------------- |
| Progress       | `...0001`   | Read + Notify | 8 bytes: mountain, sessions, streaks  |
| Command        | `...0003`   | Write         | `0x01` = log activity, `0x02` = reset |

### Progress Payload (8 bytes)

| Byte | Field             |
| ---- | ----------------- |
| 0    | Mountain index    |
| 1    | Sessions done     |
| 2    | Sessions needed   |
| 3    | Summits reached   |
| 4    | Current streak    |
| 5    | Best streak       |
| 6-7  | Total sessions    |

---

## Mountains

| Index | Name              | Sessions | Unlocks After |
| ----- | ----------------- | -------- | ------------- |
| 0     | Colline Locale    | 3        | 0 summits     |
| 1     | Petit Sommet      | 3        | 1 summit      |
| 2     | Mont Entrainement | 3        | 2 summits     |

---

## Flash Budget

The Arduino Uno WiFi Rev.2 has 48,640 bytes of flash. ArduinoBLE alone uses ~90%. The firmware compiles at **93%** by using direct PWM servo control instead of the Servo library (saves ~1KB).

---

## Browser Support

Chrome on Android, Windows, macOS, or Linux. iOS does not support Web Bluetooth.
