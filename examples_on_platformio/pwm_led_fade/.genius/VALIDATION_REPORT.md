# Genius Validation Report - PWM LED Fade

**Developed by Amine Saoud ibn al-Bashir.**

- Generator: NanoKit Genius Generator 1.0.0
- Command: `python tools/genius/generate.py --project examples_on_platformio/pwm_led_fade`
- Generated: 2026-10-25 22:50:46 UTC
- Board registry: `registry/nanokit-esp32.yaml` (NanoKit Integrated ESP32 V2020 SMD/THT)
- Result: **PASSED** (0 errors, 0 warnings)

## Inputs

- `examples_on_platformio/pwm_led_fade/nanokit.yaml` · sha256 ae14e787f237
- `examples_on_platformio/pwm_led_fade/src/main.cpp` · sha256 904ec76e7774

## Hardware Classification

- Confirmed: External LED + 220 ohm resistor, Onboard LED
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
| `LED_PIN` | 26 | 26 | examples_on_platformio/pwm_led_fade/src/main.cpp:5 | match |
| `BUILTIN_LED_PIN` | 2 | 2 | examples_on_platformio/pwm_led_fade/src/main.cpp:6 | match |

## Known Limitations

- The generator proves consistency between manifest, registry, and source constants. It does not prove that the hardware was bench-tested; the `status:` field is a human statement of evidence.
- Flowcharts describe algorithms and remain hand-authored in `docs/Implementation_Guide.md`.
