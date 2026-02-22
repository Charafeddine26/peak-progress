# Mobile App — Peak Progress

## Platform Choice: MIT App Inventor

MIT App Inventor was chosen for rapid prototyping with BLE support.  
Reference docs: `MIT_App_Inventor_Basic_Connection.pdf` in project root.

## BLE Connection Setup

1. Use the **BluetoothLE** extension (built into App Inventor)
2. Scan for device named `"PeakProgress"`
3. Service UUID: `19B10000-E8F2-537E-4F6C-D104768A1214`
4. See `docs/BLE_protocol.md` for full characteristic spec

## Screens to Implement

### 1. Connection Screen

- Scan button → list nearby BLE devices
- Auto-connect to "PeakProgress" when found
- Show connection status (connected/disconnected)

### 2. Home Dashboard

- Current mountain name (from Mountain characteristic)
- Progress bar: sessions / total sessions
- Week X of Y (calculated: `ceil(sessions / 7)` / `weeksRequired`)
- Streak counter
- Total summits count
- "Log Activity" button → write `0x01` to Command characteristic

### 3. Mountain Library

- List all 9 mountains in tier groups
- Show status per mountain: 🔒 Locked / 🔓 Unlocked / ▶ In Progress / ✅ Completed
- Read from Unlock characteristic bitfield to determine status

### 4. Progress History

- Simple list of total stats
- Calendar view showing active days (future enhancement)

### 5. Settings

- Reset button → write `0x02` to Command (with confirmation dialog)
- Device connection info

## Data Flow

```
App connects → Reads Progress + Mountain + Unlock characteristics
App subscribes to Notify on all three
User taps device → Arduino notifies → App UI updates automatically
User taps "Log Activity" in app → Writes 0x01 → Arduino fires logActivity()
```

## Building the App

1. Go to https://ai2.appinventor.mit.edu
2. Create new project "PeakProgress"
3. Add BluetoothLE component (non-visible)
4. Build screens per spec above
5. Export `.aia` file to `mobile-app/PeakProgress.aia`
