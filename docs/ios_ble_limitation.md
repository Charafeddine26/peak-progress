# iOS Web Bluetooth Limitation

## Problem Summary

The Peak Progress web app uses the **Web Bluetooth API** to connect to the Arduino over BLE. This API is **not supported on iOS** — not in Safari, and not in Chrome or any other browser either, because Apple forces all iOS browsers to use the WebKit rendering engine, which does not expose Web Bluetooth.

This means iPhone users cannot use the web companion app to view progress or log activity via the phone interface.

---

## Why This Happens

| Platform        | Browser        | Web Bluetooth | Notes                                              |
| --------------- | -------------- | :-----------: | -------------------------------------------------- |
| Android         | Chrome         | ✅ Works      | Full support                                       |
| Windows / macOS | Chrome / Edge  | ✅ Works      | Full support                                       |
| Linux           | Chrome         | ✅ Works      | Full support (requires Bluetooth permissions)      |
| iOS (iPhone)    | Safari         | ❌ Blocked    | Apple has not implemented Web Bluetooth in WebKit  |
| iOS (iPhone)    | Chrome for iOS | ❌ Blocked    | Forced to use WebKit — same restriction as Safari  |
| iOS (iPhone)    | Firefox for iOS| ❌ Blocked    | Same — all iOS browsers share WebKit engine        |

This is an Apple platform decision, not a bug in the app. It affects all Web Bluetooth applications globally.

---

## Options Considered

### Option 1 — Bluefy (Third-Party BLE Browser)
[Bluefy](https://apps.apple.com/app/bluefy-web-ble-browser/id1492822055) is an App Store browser that adds Web Bluetooth support to iOS via a custom engine. The existing web app would work through it with **zero code changes**.

**Problem:** Requires users to download a separate app they wouldn't otherwise have, adds friction, not a transparent solution.

### Option 2 — React Native App
A proper cross-platform app using `react-native-ble-plx` could support iOS + Android natively.

**Problem:** Publishing to the App Store requires an Apple Developer account ($99/year), an App Store review process (days to weeks), and significant additional development effort.

### Option 3 — Flutter App
Same concept with `flutter_blue_plus`. Same App Store publishing constraints as React Native.

### Option 4 — Accept the Limitation
Since the **physical capacitive touch button is the primary interaction** and the phone app is a supplementary display, iOS support is a nice-to-have, not a core requirement. The limitation can be documented clearly.

---

## Current Decision

**Option 4 — Accepted as a known platform limitation.**

The web companion app is documented as:
> Compatible with **Android** and **desktop Chrome/Edge**. iOS is not supported due to Apple's platform-level restriction on the Web Bluetooth API.

For demo purposes, use an Android device or a laptop. The physical button works for all users regardless of what phone they have.

---

## Future Resolution Paths

If iOS support becomes a hard requirement in a future iteration, the recommended approach would be:

1. **TestFlight distribution** — Build a React Native or Flutter app and distribute via Apple TestFlight (bypasses App Store review, free with Apple Developer account). Suitable for internal team testing.
2. **Capacitor/Ionic hybrid** — Wrap the existing web app in a native shell with Capacitor's BLE plugin. Less work than a full rewrite, but still requires native build tooling and TestFlight distribution.

---

*Documented: 2026-03-01*
