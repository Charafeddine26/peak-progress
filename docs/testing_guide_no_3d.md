# 🧪 Testing Guide — No 3D Parts Required

> Test the full Peak Progress firmware using only the raw electronics on your desk.
> No mountain shell, no climber figure, no enclosure needed.

---

## What You Need

| Item                            | You Have It? | Notes               |
| ------------------------------- | :----------: | ------------------- |
| Arduino Uno WiFi Rev.2          |      ✅      | From kit            |
| Adafruit Circuit Playground     |      ✅      | From kit            |
| SG90 Micro Servo                |      ✅      | From Grove kit      |
| USB cables (2)                  |      ✅      | One per board       |
| Jumper wires (3-5)              |      ✅      | For servo + serial  |
| Computer with Arduino IDE       |      ✅      |                     |
| Smartphone with nRF Connect app |      ✅      | Free on iOS/Android |
| Grove OLED 0.96" (optional)     |      ✅      | From Grove kit      |

---

## Step 0: Install Arduino Libraries

Open **Arduino IDE → Sketch → Include Library → Manage Libraries**, and install:

- `ArduinoBLE` (for Uno WiFi Rev.2)
- `Adafruit Circuit Playground` (for Circuit Playground)
- `U8g2` (only if using OLED)

---

## Step 1: Enable Fast Test Mode

Before uploading, enable rapid testing in `firmware/arduino_main/progress.h`:

```cpp
// Line 23 — UNCOMMENT this:
#define FAST_TEST_MODE
```

This reduces every mountain from 7-28 sessions down to **3 sessions each**, so you can test the entire 9-mountain progression in ~27 taps instead of 133.

---

## Step 2: Minimal Wiring (Breadboard on Desk)

No mountain needed. Just connect the boards on your desk:

```
Arduino Uno WiFi Rev.2          Circuit Playground
┌──────────────────┐            ┌──────────────────┐
│                  │            │                  │
│  D1 (TX) ════════╪════════════╪═══ RX            │
│  D0 (RX) ════════╪════════════╪═══ TX            │
│  5V ═════════════╪══════╦═════╪═══ VOUT/3.3V     │
│  GND ════════════╪══════╬═════╪═══ GND           │
│                  │      ║     │                  │
│  D9 ═════╗       │      ║     └──────────────────┘
│          ║       │      ║
└──────────║───────┘      ║     SG90 Servo
           ║              ║     ┌──────────────┐
           ╚═══════════════╬════╪ Orange (Sig) │
                           ╠════╪ Red (5V)     │
                           ╚════╪ Brown (GND)  │
                                └──────────────┘
```

> **Tip:** The servo will just spin freely on the desk — that's expected. You'll see it rotate as you tap, which proves the motor logic works. Later it drives the climber via the pulley.

### Optional: Grove OLED

Plug the Grove OLED into **A4/A5** (I2C) and uncomment `#define USE_OLED` in `display.h`.

---

## Step 3: Upload Firmware

### 3A: Upload to Circuit Playground

1. Plug Circuit Playground into USB
2. **Disconnect the serial wires** (D0/D1) from the Arduino — Arduino IDE uses those same pins for USB upload and they'll conflict
3. Arduino IDE: **Tools → Board → Adafruit Circuit Playground**
4. Arduino IDE: **Tools → Port → (select CP port)**
5. Open `firmware/circuit_playground/circuit_playground.ino`
6. Click **Upload** ✓
7. You should see a brief green flash on all 10 LEDs = boot success

### 3B: Upload to Arduino Uno WiFi Rev.2

1. Plug Arduino into USB
2. **Disconnect serial wires** (D0/D1) temporarily
3. Arduino IDE: **Tools → Board → Arduino Uno WiFi Rev2**
4. Arduino IDE: **Tools → Port → (select Arduino port)**
5. Open `firmware/arduino_main/arduino_main.ino`
6. Click **Upload** ✓

### 3C: Reconnect Serial Wires

After both uploads succeed, **reconnect D0↔TX and D1↔RX** between the boards.

> ⚠️ **Critical:** Always disconnect serial wires before uploading. If you forget, the upload will fail with a timeout error.

---

## Step 4: Open Serial Monitor (Debug Output)

1. With Arduino connected to USB, open **Tools → Serial Monitor**
2. Set baud rate to **9600**
3. You should see boot messages indicating the loaded mountain and progress state

---

## Step 5: Test Sequence

### Test A: First Tap (Activity Logging)

| Do This                                                             | Expect This                                               |
| ------------------------------------------------------------------- | --------------------------------------------------------- |
| **Touch pad #3** on Circuit Playground (the copper pad labeled "3") | Brief white flash on all LEDs                             |
|                                                                     | Servo rotates ~50° from base position (1/3 of the way up) |
|                                                                     | First LED lights up in **blue** (Colline Locale palette)  |
|                                                                     | A rising tone plays from CP buzzer                        |
|                                                                     | Serial monitor shows session count = 1                    |

### Test B: Second Tap

| Do This                                         | Expect This                    |
| ----------------------------------------------- | ------------------------------ |
| **Touch pad #3 again** (wait 1.5s for debounce) | Servo moves further up (~100°) |
|                                                 | Second LED lights up           |
|                                                 | Slightly higher tone plays     |

### Test C: Summit! (Third Tap in Fast Test Mode)

| Do This                       | Expect This                                                                        |
| ----------------------------- | ---------------------------------------------------------------------------------- |
| **Touch pad #3** a third time | Servo moves to 170° (summit position)                                              |
|                               | **Summit celebration:** Green pulse animation on all LEDs                          |
|                               | 3-note victory melody plays (C-E-G)                                                |
|                               | Serial monitor: "Summit reached!"                                                  |
|                               | After 3 seconds: servo descends back to 20° (base camp)                            |
|                               | LEDs clear and re-light with **new color palette** (Petit Sommet: blue-cyan-green) |
|                               | Serial monitor: next mountain = "Petit Sommet"                                     |

### Test D: Power Cycle (EEPROM Persistence)

| Do This                          | Expect This                                                        |
| -------------------------------- | ------------------------------------------------------------------ |
| **Unplug the Arduino USB cable** | Everything powers off                                              |
| **Wait 5 seconds**               |                                                                    |
| **Plug it back in**              | Servo moves to its **last saved position**                         |
|                                  | LEDs restore to the correct mountain's palette                     |
|                                  | Serial monitor shows the same mountain and session count as before |

### Test E: Full Progression (9 Mountains)

Tap through all 9 mountains (3 taps each = 27 total taps). Observe:

| Mountain # | Name                | Taps | LED Colors               | Melody            |
| :--------: | ------------------- | :--: | ------------------------ | ----------------- |
|     1      | Colline Locale      |  3   | Blue → Green             | 3-note simple     |
|     2      | Petit Sommet        |  3   | Blue → Cyan → Green      | 3-note simple     |
|     3      | Mont d'Entraînement |  3   | Purple → Gold            | 4-note moderate   |
|     4      | Mont Blanc          |  3   | Ice Blue → White → Gold  | 5-note triumphant |
|     5      | Matterhorn          |  3   | Dark Blue → Purple → Red | 5-note triumphant |
|     6      | Kilimanjaro         |  3   | Orange → Gold → Red      | 6-note epic       |
|     7      | Denali              |  3   | White → Blue             | 6-note epic       |
|     8      | Everest             |  3   | Rainbow                  | 8-note legendary  |
|     9      | K2                  |  3   | Red → Orange → Yellow    | 8-note legendary  |

After K2, it wraps back to mountain 0.

### Test F: BLE (Phone Connection)

1. Install **nRF Connect** (free) on your phone
2. Open the app → **SCANNER** tab → look for **"PeakProgress"**
3. Tap **CONNECT**
4. Expand the service `19B10000-...`
5. Tap the **↓ read** arrow on the first characteristic (Progress) → you'll see 8 bytes of hex data
6. Tap the **↑ write** arrow on the Command characteristic → write value `01` → the Arduino fires `logActivity()` as if you tapped the pad!
7. Read Progress again → the session count byte has incremented

### Test G: Reset

1. In nRF Connect, write `02` to the Command characteristic
2. Servo returns to base camp, LEDs clear, progress resets to mountain 0
3. Verify on Serial Monitor: all counters back to 0

---

## Troubleshooting

| Problem                     | Fix                                                                                                                                                                 |
| --------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Upload fails with timeout   | **Disconnect serial wires** (D0/D1) before uploading                                                                                                                |
| No BLE device found         | Ensure `ArduinoBLE` library is installed; Arduino Uno WiFi **Rev.2** required (not original Uno)                                                                    |
| Touch doesn't register      | Try pressing pad #3 firmly with your finger (needs skin contact, not nail). Adjust `TOUCH_THRESHOLD` in `circuit_playground.ino` if needed (lower = more sensitive) |
| Servo jitters at rest       | Normal for cheap servos. Firmware detaches servo after each move to minimize this                                                                                   |
| LEDs don't change color     | Check serial wiring (TX→RX, RX→TX — they cross!)                                                                                                                    |
| OLED shows nothing          | Ensure `#define USE_OLED` is uncommented in `display.h` and `U8g2` library is installed                                                                             |
| Double-tap registers as one | Debounce is 1.5 seconds — wait longer between taps                                                                                                                  |

---

## What This Proves (Without the 3D Mountain)

✅ Servo moves proportionally to session progress → **climber movement works**
✅ LEDs change palette per mountain → **altitude visualization works**
✅ Summit triggers celebration animation + melody → **reward system works**
✅ Progress survives power cycles → **EEPROM persistence works**
✅ 9-mountain unlock chain → **progression system works**
✅ BLE read/write/notify → **mobile app communication works**
✅ Touch detection with debounce → **user input works**

Once the 3D mountain is printed, you just mount these same electronics inside it — the firmware is already proven.
