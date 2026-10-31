# Genius Validation Report - Deep Sleep Wakeup

**Developed by Amine Saoud ibn al-Bashir.**

- Generator: NanoKit Genius Generator 1.0.0
- Command: `python tools/genius/generate.py`
- Generated: 2026-10-31 23:35:27 UTC
- Board registry: `registry/nanokit-esp32.yaml` (NanoKit Integrated ESP32 V2020 SMD/THT)
- Result: **PASSED** (0 errors, 0 warnings)

## Inputs

- `examples_on_platformio/deep_sleep_wakeup/nanokit.yaml` · sha256 f1238810f36e
- `examples_on_platformio/deep_sleep_wakeup/src/main.cpp` · sha256 0e991cc2f403

## Hardware Classification

- Confirmed: Wake button + 10 kOhm pull-down, Onboard LED
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
| `WAKE_BUTTON_PIN` | 33 | 33 | examples_on_platformio/deep_sleep_wakeup/src/main.cpp:7 | match |
| `BUILTIN_LED_PIN` | 2 | 2 | examples_on_platformio/deep_sleep_wakeup/src/main.cpp:8 | match |

## Known Limitations

- The generator proves consistency between manifest, registry, and source constants. It does not prove that the hardware was bench-tested; the `status:` field is a human statement of evidence.
- Flowcharts describe algorithms and remain hand-authored in `docs/Implementation_Guide.md`.
