# Testing - NanoKit Drone 4X

**Developed by Amine Saoud ibn al-Bashir.**

## Bench Gate

Propellers remain removed for every test in this document.

| Stage | Test | Required result |
|---|---|---|
| 1 | Flight firmware build | PlatformIO build succeeds without warnings that hide errors |
| 2 | Camera-node build | Build succeeds with all unverified payload features disabled |
| 3 | Boot outputs | M1-M4 stay at 1000 us with ESC confirmation gate at `0` |
| 4 | SoftAP and HTTP | `NanoKit-Drone-4X` appears and `http://192.168.4.1/api/status` responds |
| 5 | WebSocket | HELLO, ACK, and truthful TEL packets follow protocol v3 |
| 6 | Stale command | Armed-state simulation enters failsafe after 600 ms |
| 7 | Emergency stop | Failsafe latches and every output returns to minimum |
| 8 | Missing IMU | Arming remains blocked; telemetry marks IMU and attitude invalid |
| 9 | ESC gate | Arming remains blocked while analogue PWM is unconfirmed |
| 10 | UI responsive layout | No overlapping safety controls on desktop, tablet, or mobile |

## Sensor Driver Gate

For each future sensor, test actual address discovery, units, rate, stale-data rejection, disconnect/reconnect, bus lockup, and invalid-value handling. The UI must never keep a stale value marked valid after a read failure.

## ESC Gate

Use an oscilloscope or logic analyzer to verify frequency and pulse widths before connecting ESC signal inputs. Confirm the ESC manual accepts analogue PWM. Then test one ESC at a time with propellers removed and a current-limited supply. Only after documented success may the compile-time confirmation gate be reviewed.

## No Automatic Flight Readiness

Passing software builds and bench checks does not authorize tethered or free flight. Structural, RF, battery, propulsion, control-law, vibration, and local regulatory reviews are separate requirements.
