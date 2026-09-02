# Open-Source Project Engineering Standard

**Developed by Amine Saoud ibn al-Bashir.**

This standard applies to every new or substantially extended project inside `open_source_projects/`. It establishes the expected engineering package and does not claim unattended automation.

## Project Package

Each implemented project should be self-contained inside its project directory and normally include:

- `README.md` with purpose, scope, architecture, features, safety, build, upload, and usage.
- Project-owned firmware, applications, internal libraries, configuration, and tests.
- Hardware inventory and bill of materials.
- Complete NanoKit Pin to ESP32 GPIO to Module Pin connection tables.
- Wiring order, power distribution, grounding, protection, and power-budget documentation.
- Communication-bus maps for I2C, SPI, UART, PWM, I2S, SD, camera, audio, network, or control links that are actually used.
- Implementation guide, algorithm, pseudocode, initialization sequence, failure behavior, and troubleshooting.
- Standalone bring-up test and acceptance criteria for every component.
- Mermaid system, physical connection, power, bus, firmware-flow, boot, and failsafe diagrams as applicable.
- Project license and contribution information when required by the project.
- A truthful `.genius/README.md` when generated engineering output is reserved.

Use `examples_on_platformio/ultrasonic_distance/` as the minimum documentation-detail benchmark for individual hardware connections, then expand it to match the project's complete architecture.

## Hardware Evidence

Classify every component as **Confirmed**, **Proposed**, or **Blocked/TODO**. Never invent pins, addresses, voltages, limits, module revisions, power data, or electrical behavior.

For every Confirmed component, document:

- Exact module name and revision, quantity, purpose, and evidence source.
- Module pin, signal direction, protocol, voltage, and logic level.
- NanoKit physical pin and ESP32 GPIO.
- I2C address or multiplexer channel, SPI CS, UART baud, PWM limits, or equivalent configuration.
- Pull-ups, level shifting, voltage dividers, isolation, protection, and common ground.
- Power source, estimated current, mounting location, and axis orientation.
- Driver status, Feature Flag, bring-up result, and validity gate.

A Proposed or Blocked/TODO item must state what is missing and remain disabled behind a Feature Flag until the evidence and test pass.

## System Integration

Document ownership of every actuator and data source, startup order, communication contracts, timeouts, reconnection, degraded modes, safe states, and emergency behavior. Safety-critical outputs must remain disabled during incomplete integration.

For repeated I2C addresses, document verified multiplexer channels or re-addressing. Verify the exact ESP32 module and runtime PSRAM before relying on PSRAM. Confirm motor-controller and actuator signal limits before enabling physical outputs.

## Genius Guides

The `.genius/` directory is only a destination for generated engineering guides, checklists, summaries, and validation reports. It is not an automatic system by itself. Every generated artifact must state the generator, command, inputs, generation time, and validation result.

## Repository Registration

After explicitly creating, removing, or renaming an open-source project, run:

```powershell
python tools/update_repository_index.py
```

Do not run registration for ordinary project edits that leave the project list unchanged.

## Validation

Build all affected targets, run available unit, integration, and documentation tests, inspect `git diff`, and verify scope with `git diff --name-only`. Report every modified file and separate Confirmed, Proposed, and Blocked/TODO hardware. Never commit or push unless explicitly requested.
