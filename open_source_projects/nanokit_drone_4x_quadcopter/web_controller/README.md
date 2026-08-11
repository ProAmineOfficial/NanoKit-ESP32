# NanoKit Drone 4X Flight Deck

**Developed by Amine Saoud ibn al-Bashir.**

The Flight Deck is a responsive browser Ground Control Station for the NanoKit Drone 4X Wi-Fi/WebSocket link. It provides dual-stick commands, hold-to-arm, disarm, emergency stop, mode gates, mission planning, payload status, and real telemetry presentation.

## Onboard Use

The firmware serves this directory from LittleFS:

```powershell
cd firmware
pio run -t uploadfs
```

1. Join Wi-Fi network `NanoKit-Drone-4X` with password `NanoKit4X`.
2. Open `http://192.168.4.1`.
3. Select **Connect Flight Link**.

The page opens `ws://192.168.4.1:81` and sends protocol-v3 commands at 10 Hz. The firmware disarms when commands are stale. The browser never writes directly to ESC pins.

## Local UI Preview

```powershell
python -m http.server 8080 --directory web_controller
```

Open `http://127.0.0.1:8080`. The interface can be inspected locally, but a live flight link still requires the NanoKit access point. HTTPS pages cannot open an insecure `ws://` device link because browsers block mixed content.

## Telemetry Rules

- Values are shown only when their corresponding validity field is true.
- Missing IMU, barometer, power, GNSS, optical-flow, or obstacle data stays visibly unavailable.
- Disabled payload features remain disabled in the UI.
- Mission upload remains locked until navigation hardware and firmware are verified.

See [Wi-Fi/WebSocket Protocol](../docs/WiFi_WebSocket_Protocol.md) and [Safety](../docs/Safety.md).
