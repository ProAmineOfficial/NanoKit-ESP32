# Wi-Fi and WebSocket Protocol - NanoKit Drone 4X

**Developed by Amine Saoud ibn al-Bashir.**

## Endpoints

| Service | Address |
|---|---|
| Wi-Fi SoftAP | `NanoKit-Drone-4X` |
| Default password | `NanoKit4X` |
| Flight Deck | `http://192.168.4.1` |
| Status API | `http://192.168.4.1/api/status` |
| Command and telemetry socket | `ws://192.168.4.1:81` |

Change the default password before any field test. The current link is intended for controlled bench development, not secure long-range operation.

## Command Packet

Commands are UTF-8 text with semicolon-separated `KEY=VALUE` fields:

```text
PROTO=3;TYPE=CMD;SEQ=42;T=0;R=0;P=0;Y=0;A=0;CAL=0;STOP=0;MODE=MANUAL;
```

| Field | Meaning | Firmware limit |
|---|---|---|
| `SEQ` | Increasing command sequence | Must be above zero |
| `T` | Throttle command | 0-1000 |
| `R`, `P` | Roll and pitch targets | -25 to +25 degrees |
| `Y` | Yaw-rate target | -120 to +120 deg/s |
| `A` | Arm request | `0` or `1` |
| `CAL` | IMU calibration request | `0` or `1` |
| `STOP` | Emergency-stop request | `0` or `1` |
| `MODE` | Flight mode | `MANUAL`, `ALTITUDE_HOLD`, `POSITION_HOLD`, `MISSION` |

The parser clamps numeric fields. Invalid protocol version, type, or sequence is rejected.

## Responses

On connection:

```text
PROTO=3;TYPE=HELLO;DEVICE=NanoKit-Drone-4X;TRANSPORT=WIFI;SAFETY=LOCKED;
```

For an accepted command:

```text
PROTO=3;TYPE=ACK;SEQ=42;
```

Telemetry packets include the safety state, acknowledged command, command age, sensor-validity flags, real measurements when valid, and four output pulse values. The Flight Deck must never treat a numeric field as valid without its corresponding validity flag.

## Failsafe Behaviour

The flight controller disarms an armed system when the client disconnects or a current command is not received within 600 ms. Reconnection does not re-arm automatically. The pilot must release ARM, keep throttle at zero, clear the fault, and perform a new hold-to-arm action.
