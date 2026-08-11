# Open-Source References - NanoKit Drone 4X

**Developed by Amine Saoud ibn al-Bashir.**

This project is an original NanoKit integration and teaching reference. Established open-source flight-control projects may be studied for architecture, estimator design, safety review, test strategy, and protocol ideas, but their code and gains must not be copied without checking license compatibility and hardware assumptions.

Useful upstream categories include:

- Espressif Arduino-ESP32 and ESP-IDF documentation for ESP32 peripherals, FreeRTOS, Wi-Fi, and PSRAM.
- PlatformIO documentation for reproducible builds and filesystem uploads.
- PX4 and ArduPilot documentation for mature safety concepts and flight-stack architecture.
- Betaflight documentation for rate control, mixer concepts, and ESC protocol context.
- Individual sensor manufacturer datasheets for electrical limits, addresses, timing, and calibration.

## Reuse Checklist

1. Record the source URL, version or commit, and license.
2. Confirm the license is compatible with this repository.
3. Document every adaptation to NanoKit pins, voltage, task timing, and safety state.
4. Add tests that prove the adapted behaviour rather than assuming upstream hardware equivalence.
5. Preserve required notices and attribution.

No reference source overrides the local rule that unverified hardware remains disabled.
