# Legacy Bluetooth Protocol Note - NanoKit Drone 4X

**Developed by Amine Saoud ibn al-Bashir.**

This filename is retained so existing repository links do not break. The active NanoKit Drone 4X controller no longer uses browser Web Bluetooth or BLE GATT.

## Current Transport

- NanoKit creates a Wi-Fi SoftAP.
- The Flight Deck is served at `http://192.168.4.1`.
- Commands and telemetry use WebSocket protocol v3 on port `81`.
- A stale or disconnected command link triggers the firmware failsafe.

See [Wi-Fi and WebSocket Protocol](WiFi_WebSocket_Protocol.md) for the active packet format.

## Why The Old Path Was Removed

The browser control link, onboard UI hosting, payload networking, and deterministic safety reporting are easier to test as one versioned Wi-Fi/WebSocket contract. Classic Bluetooth pairing entries and BLE GATT characteristics are not part of this revision. Removing those dependencies also avoids presenting Windows pairing status as proof that the flight command protocol is alive.

Future Bluetooth support, if added, must be an independent transport adapter feeding the same validated `PilotCommand` queue. It must not bypass freshness, arming, emergency-stop, or ESC confirmation gates.
