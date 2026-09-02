# PlatformIO Application Engineering Standard

**Developed by Amine Saoud ibn al-Bashir.**

This standard applies to every new or substantially extended application inside `applications_on_platformio/`. It defines the expected project package; it does not run automatically in the background.

## Application Package

A complete application should keep all implementation and documentation inside its own project path and normally include:

- A root `README.md` describing the problem, architecture, features, hardware, software, and operation.
- Reproducible PlatformIO configuration and project-owned source code for each firmware target.
- Project-local wiring and implementation guides for each hardware node.
- Communication contracts for Wi-Fi, BLE, WebSocket, UART, I2C, SPI, or other links.
- User-interface instructions when a web, desktop, or mobile controller exists.
- Bring-up tests, integration tests, failure behavior, safety rules, and troubleshooting.
- Mermaid connection, architecture, power, bus, boot, firmware-flow, and safety diagrams as applicable.
- A truthful `.genius/README.md` when the project reserves space for generated engineering guides.

Use `examples_on_platformio/ultrasonic_distance/` as the minimum detail benchmark for each hardware-facing node, while adapting the material to the application's real architecture.

## Hardware Evidence

Every component must be classified as **Confirmed**, **Proposed**, or **Blocked/TODO**. Never invent NanoKit pins, ESP32 GPIOs, voltages, addresses, bus limits, module revisions, or electrical properties.

For every Confirmed component, record:

- Exact module and revision, purpose, and evidence source.
- Module pin, signal direction, protocol, supply voltage, and logic voltage.
- NanoKit physical pin and ESP32 GPIO.
- I2C address or TCA9548A channel, SPI CS, UART baud, PWM limits, or equivalent configuration.
- Pull-ups, level shifting, protection, common ground, power source, and estimated current.
- Mounting position, axis orientation, driver status, and Feature Flag when relevant.

Proposed and Blocked/TODO components must state exactly what evidence is missing and remain disabled until verification succeeds.

## Integration Documentation

Document every node and the links between nodes. Include data ownership, message formats, connection sequence, timeouts, reconnection behavior, validity gating, and the safe state entered when a link or sensor fails.

When repeated bus addresses exist, document the verified re-addressing sequence or multiplexer channels. Treat PSRAM as module-level hardware and verify the exact ESP32 module and runtime PSRAM size instead of proposing breadboard wiring.

## Genius Guides

The `.genius/` directory is an optional destination for generated checklists, summaries, and validation reports. It is not automation by itself. A generated file must record the generator, command, inputs, and generation time.

## Repository Registration

Run `python tools/update_repository_index.py` only after explicitly creating, removing, or renaming an application. Ordinary application edits do not require repository registration.

## Validation

Build every affected firmware target, run available application and documentation tests, inspect `git diff`, and verify scope with `git diff --name-only`. Report modified files and hardware status groups separately. Never commit or push unless explicitly requested.
