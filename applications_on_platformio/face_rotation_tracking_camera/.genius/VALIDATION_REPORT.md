# Genius Validation Report - Face Rotation Tracking Camera

**Developed by Amine Saoud ibn al-Bashir.**

- Generator: NanoKit Genius Generator 1.0.0
- Command: `python tools/genius/generate.py`
- Generated: 2026-11-01 00:07:34 UTC
- Board registry: `registry/nanokit-esp32.yaml` (NanoKit Integrated ESP32 V2020 SMD/THT)
- Result: **PASSED** (0 errors, 0 warnings)

## Inputs

- `applications_on_platformio/face_rotation_tracking_camera/esp32/src/main.cpp` · sha256 7916577df61c
- `applications_on_platformio/face_rotation_tracking_camera/nanokit.yaml` · sha256 4ac7b84bf3eb

## Hardware Classification

- Confirmed: Pan servo, Tilt servo, Onboard status LED
- Proposed: none
- Blocked/TODO: none

## Rules Checked

flash-pin (GPIO6-11), input-only (GPIO34/35/36/39), uart0-shared (GPIO1/3), strapping (GPIO0/2/12/15), adc2-wifi, dac-capable, 5v-input level shifting, double-assigned GPIO, header-mismatch, i2c-collision, confirmed-needs-gpio, evidence (manifest GPIO vs firmware constant).

## Findings

| Level | Rule | Where | Message |
|---|---|---|---|
| info | strapping | builtin_led.LED | GPIO2 (NanoKit Pin 17): must be low or floating at boot for download mode; the onboard LED and a signal output are acceptable loads. |

## Code Evidence

| Symbol | Manifest GPIO | Code value | Location | Status |
|---|---:|---:|---|---|
| `PAN_SERVO_PIN` | 26 | 26 | applications_on_platformio/face_rotation_tracking_camera/esp32/src/main.cpp:8 | match |
| `TILT_SERVO_PIN` | 25 | 25 | applications_on_platformio/face_rotation_tracking_camera/esp32/src/main.cpp:9 | match |
| `STATUS_LED_PIN` | 2 | 2 | applications_on_platformio/face_rotation_tracking_camera/esp32/src/main.cpp:10 | match |

## Known Limitations

- The generator proves consistency between manifest, registry, and source constants. It does not prove that the hardware was bench-tested; the `status:` field is a human statement of evidence.
- Flowcharts describe algorithms and remain hand-authored in `docs/Implementation_Guide.md`.
