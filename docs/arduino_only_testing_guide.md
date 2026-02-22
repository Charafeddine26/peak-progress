# 🚀 Arduino-Only Testing Guide

> If you don't have the Circuit Playground available right now, you can still test **90% of the project's logic** using just the Arduino Uno WiFi Rev.2, the Servo, and your phone (via BLE).

Since the Circuit Playground is missing, you won't have:

- Capacitive touch input
- LED progress lights
- Audio feedback

**BUT you still have:**

- ✅ The entire Peak Progress mountain unlocking logic
- ✅ EEPROM saving/loading
- ✅ Servo mountain climbing movement
- ✅ BLE communication with your phone

Your phone will act as the "touch button" for this week!

---

## Step 1: Enable Fast Test Mode

Before we start, let's make sure testing goes quickly. In `firmware/arduino_main/progress.h`:

```cpp
// Line 23 — UNCOMMENT this so each mountain only takes 3 sessions:
C#define FAST_TEST_MODE
```

---

## Step 2: Wiring (Arduino + Servo Only)

Connect your SG90 Servo motor to the Arduino shield using the white connectors:

1. Look for the white plug labeled **D8** (it's in the middle row, second from the left in your photo).
2. The servo cable (with Orange/Red/Brown wires) needs to plug into this white **D8** connector.

_Wait! The code says D9, but the shield has D8 and D9 together?_
Yes! On these Grove shields, the connector labeled "D8" actually has both pin 8 and pin 9 inside it (the inside pins are D8, D9, VCC, GND). So plugging the servo into the **D8** plug ensures its signal wire connects to Arduino pin **D9**.

```
Arduino Shield (White Plug D8)       SG90 Servo
┌──────────────────┐                 ┌──────────────┐
│                  │                 │              │
│  VCC (Pin 3) ════╪═════════════════╪══ Red Wire   │  (Power)
│  GND (Pin 4) ════╪═════════════════╪══ Brown Wire │  (Ground)
│                  │                 │              │
│  D9  (Pin 2) ════╪═════════════════╪══ Orange Wire│  (Signal)
│                  │                 └──────────────┘
└──────────────────┘
```

_(Note: Don't connect anything to the RX/TX pins. Leave them empty so uploading code always works perfectly)._

### Bonus: Grove LCD RGB Backlight

Plug the Grove LCD into any **I2C** white connector on the shield (there are 4 of them, labeled "I2C" on the left side of the board). Use the included Grove cable — no jumper wires needed.

The LCD will show:

- **Line 1:** Mountain name with a tiny mountain icon
- **Line 2:** Tier, session count, and week progress
- **Backlight color** changes per tier: 🟢 Green (Tier 1) → 🔵 Cyan (Tier 2) → 🟡 Gold (Tier 3) → 🟣 Purple (Tier 4)

**Library install:** In Arduino IDE, go to **Sketch → Include Library → Add .ZIP Library** → select `Grove_LCD_RGB_Backlight-master.zip` from the `TITT/` project root folder.

---

## Step 3: Upload the Firmware

1. Plug the Arduino Uno WiFi Rev.2 into your computer via USB.
2. Open `firmware/arduino_main/arduino_main.ino`.
3. In Arduino IDE: **Tools → Board → Arduino Uno WiFi Rev2**.
4. Click **Upload** ✓.
5. Open **Tools → Serial Monitor** (set to 9600 baud) so you can see what the Arduino is thinking. It will print messages like "Current Mountain: Colline Locale".

> _Wait, do I need to change code because the Circuit Playground is missing?_
> No! The Arduino will continuously send "change LED" and "play tone" commands to the empty serial port. It doesn't care that nobody is listening on the other end. The core logic still runs perfectly.

---

## Step 4: How to Test (Using your Phone)

Since you don't have the touch pad, you will trigger the `logActivity()` function from your smartphone using Bluetooth (BLE).

### Getting Ready

1. On iOS or Android, download the free app **nRF Connect for Mobile**.
2. Open nRF Connect and go to the **SCANNER** tab.
3. Power on the Arduino (plugged into USB).
4. In the app scanner, look for a device named **PeakProgress**. Tap **CONNECT**.

### Testing the Climb (Logging Activity)

1. Once connected, tap the `Unknown Service` that ends in `1214`.
2. You will see several characteristics. Look for the one that has an **↑ Up Arrow** (this is the Command characteristic, ending in `0003`).
3. Tap the **↑ Up Arrow**.
4. A box will pop up. Choose **BYTE** or **UINT8** as the data type.
5. Enter the value `01` (or just `1`) and hit **Send/Write**.

**What happens:**

- The Arduino receives the `01` command via Bluetooth.
- It triggers the exact same logic as if you had touched the Circuit Playground pad!
- You will see the **Servo motor rotate** a few degrees upwards.
- Look at the Arduino IDE Serial Monitor — you'll see "Session logged: 1/3".

### Testing the Summit & Progression

1. Send the `01` command two more times.
2. On the 3rd time, the Arduino reaches the summit!
3. Watch the Servo: it will hold at the top for 3 seconds (celebration time), and then slowly descend back to the start position.
4. Check the Serial Monitor: it will announce you've unlocked the next mountain ("Petit Sommet")!

### Testing the Memory (EEPROM)

1. Send a few `01` commands so the climber is halfway up a mountain.
2. **Unplug the Arduino USB cable** entirely (simulating a power loss).
3. Plug it back in.
4. Watch the Servo — it will immediately jump right back to the halfway point it was saved at. Progress is saved!

---

## Summary for the Week

For the next week, your phone is the remote control for your mountain climber. Whenever you want to test the progression, just connect via nRF Connect and send the `01` byte. You can test the servo's physical range, the EEPROM saving logic, and the mountain unlock sequence completely independently of the Circuit Playground!
