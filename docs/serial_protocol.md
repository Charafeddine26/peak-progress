# Serial Protocol — Arduino ↔ Circuit Playground

**Baud rate:** 9600  
**Line terminator:** `\n` (newline)

## Arduino → Circuit Playground (Commands)

| Command | Format            | Example     | Description                        |
| ------- | ----------------- | ----------- | ---------------------------------- |
| Set LED | `L<idx>:<RRGGBB>` | `L3:00FF99` | Set LED `idx` to hex color         |
| Clear   | `C`               | `C`         | Turn off all 10 LEDs               |
| Melody  | `M<type>`         | `M2`        | Play melody type (0-4)             |
| Tone    | `T<freq>:<dur>`   | `T523:200`  | Play tone at freq Hz for dur ms    |
| Weekly  | `W`               | `W`         | Trigger weekly milestone animation |
| Summit  | `S<tier>`         | `S3`        | Trigger summit animation for tier  |

### Melody Types

| Value | Name       | Notes             | Used For         |
| ----- | ---------- | ----------------- | ---------------- |
| 0     | Simple     | 3-note (C-E-G)    | Tier 1 mountains |
| 1     | Moderate   | 4-note (C-E-G-C6) | Tier 1 advanced  |
| 2     | Triumphant | 5-note (+E6)      | Tier 2 mountains |
| 3     | Epic       | 6-note (expanded) | Tier 3 mountains |
| 4     | Legendary  | 8-note fanfare    | Tier 4 mountains |

## Circuit Playground → Arduino (Events)

| Event | Format | Description                   |
| ----- | ------ | ----------------------------- |
| Touch | `T`    | User tapped capacitive pad #3 |
