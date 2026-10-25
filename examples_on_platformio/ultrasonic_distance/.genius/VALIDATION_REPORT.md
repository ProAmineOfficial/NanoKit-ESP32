# Genius Validation Report - Ultrasonic Distance

**Developed by Amine Saoud ibn al-Bashir.**

- Generator: NanoKit Genius Generator 1.0.0
- Command: `python tools/genius/generate.py`
- Generated: 2026-10-25 20:33:13 UTC
- Board registry: `registry/nanokit-esp32.yaml` (NanoKit Integrated ESP32 V2020 SMD/THT)
- Result: **PASSED WITH WARNINGS** (0 errors, 1 warnings)

## Inputs

- `examples_on_platformio/ultrasonic_distance/nanokit.yaml` · sha256 c43793c0b92f
- `examples_on_platformio/ultrasonic_distance/src/main.cpp` · sha256 e5517c56fd3c

## Hardware Classification

- Confirmed: HC-SR04 ultrasonic sensor
- Proposed: none
- Blocked/TODO: none

## Rules Checked

flash-pin (GPIO6-11), input-only (GPIO34/35/36/39), uart0-shared (GPIO1/3), strapping (GPIO0/2/12/15), adc2-wifi, dac-capable, 5v-input level shifting, double-assigned GPIO, header-mismatch, i2c-collision, confirmed-needs-gpio, evidence (manifest GPIO vs firmware constant).

## Findings

| Level | Rule | Where | Message |
|---|---|---|---|
| info | strapping | hcsr04.TRIG | GPIO5 (NanoKit Pin 36): affects SDIO slave timing only; safe as a general output. |
| warning | 5v-input | hcsr04.ECHO | hcsr04.ECHO: 5 V signal on GPIO34 needs the declared level shifting (required). |

## Code Evidence

| Symbol | Manifest GPIO | Code value | Location | Status |
|---|---:|---:|---|---|
| `TRIGGER_PIN` | 5 | 5 | examples_on_platformio/ultrasonic_distance/src/main.cpp:8 | match |
| `ECHO_PIN` | 34 | 34 | examples_on_platformio/ultrasonic_distance/src/main.cpp:9 | match |

## Known Limitations

- The generator proves consistency between manifest, registry, and source constants. It does not prove that the hardware was bench-tested; the `status:` field is a human statement of evidence.
- Flowcharts describe algorithms and remain hand-authored in `docs/Implementation_Guide.md`.
