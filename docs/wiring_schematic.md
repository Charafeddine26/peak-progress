# Wiring Schematic — Peak Progress

## Pin Assignments

### Arduino Uno WiFi Rev.2

| Pin          | Connection                      | Notes           |
| ------------ | ------------------------------- | --------------- |
| **D0 (RX)**  | Circuit Playground TX           | Serial receive  |
| **D1 (TX)**  | Circuit Playground RX           | Serial transmit |
| **D8 (PWM)** | SG90 Servo signal (orange wire) | Servo control   |
| **5V**       | Servo VCC (red wire)            | Shared 5V rail  |
| **GND**      | Servo GND (brown wire)          | Common ground   |
| **5V**       | Circuit Playground VCC          | Power to CP     |
| **GND**      | Circuit Playground GND          | Common ground   |
| **A4 (SDA)** | Grove OLED SDA (optional)       | I2C data        |
| **A5 (SCL)** | Grove OLED SCL (optional)       | I2C clock       |

### Circuit Playground

| Pad                 | Function               | Notes                         |
| ------------------- | ---------------------- | ----------------------------- |
| **#3**              | Capacitive touch input | User tap detection            |
| **NeoPixels (0-9)** | LED altitude display   | Built-in, no wiring needed    |
| **Buzzer**          | Audio feedback         | Built-in, no wiring needed    |
| **TX**              | → Arduino RX (D0)      | Serial events                 |
| **RX**              | ← Arduino TX (D1)      | Serial commands               |
| **3.3V/VBAT**       | From Arduino 5V        | Via voltage divider if needed |
| **GND**             | Arduino GND            | Common ground                 |

## Wiring Diagram (Text)

```
┌──────────────────────┐        ┌──────────────────────┐
│   Arduino Uno WiFi   │        │  Circuit Playground  │
│                      │        │                      │
│  D1 (TX) ──────────────────── RX                    │
│  D0 (RX) ──────────────────── TX                    │
│  5V ─────────────────────┬─── VCC                   │
│  GND ────────────────────┤    │                      │
│                      │   │    │  [Pad #3] ← Touch    │
│  D8 (PWM)────┐       │   │    │  [LEDs 0-9] Built-in │
│              │       │   │    │  [Buzzer]   Built-in │
│  A4 (SDA)──┐ │       │   │    └──────────────────────┘
│  A5 (SCL)─┐│ │       │   │
└───────────┤│─┤───────┘   │
            ││ │           │
   ┌────────┘│ │    ┌──────┘
   │  ┌──────┘ │    │
   │  │        │    │
   │  │   ┌────┘    │
   │  │   │         │
┌──┴──┴┐ ┌┴─────────┴┐
│ OLED │ │  SG90 Servo │
│0.96" │ │            │
│(opt.)│ │ Orange=Sig  │
│      │ │ Red=5V      │
│SDA   │ │ Brown=GND   │
│SCL   │ └─────────────┘
└──────┘
```

## Power Notes

- **USB power** from computer or 5V USB adapter (recommended for development)
- Servo can draw up to 500mA under load — ensure adequate USB power supply
- If using battery pack: 4x AA (6V) through Arduino barrel jack, or USB power bank
