# PlatformIO Example Engineering Standard

**Developed by Amine Saoud ibn al-Bashir.**

This standard applies to every new or substantially extended project inside `examples_on_platformio/`. It is a project-authoring rule, not a background service or unattended watcher.

## Quality Benchmark

Use `examples_on_platformio/ultrasonic_distance/` as the minimum documentation-quality benchmark. Adapt its structure to the real example; do not copy sections that do not apply.

A complete example should normally contain:

- `README.md` for the goal, learning outcomes, hardware, software, build, upload, and usage.
- `platformio.ini` for a reproducible PlatformIO environment.
- `src/main.cpp` or the project-owned source files.
- `docs/Wiring.md` for connection tables, electrical notes, and wiring order.
- `docs/Implementation_Guide.md` for the algorithm, pseudocode, flowchart, tests, troubleshooting, and exercises.
- `nanokit.yaml`, the hardware manifest: every component, bus, and pin with its status and the firmware constant (`symbol:`) that carries it.
- `images/connection-diagram.md` and `images/system-diagram.md`, generated from the manifest (never edited by hand).
- Project-local `assets/`, `include/`, `lib/`, and `test/` notes when those areas are used.
- `.genius/VALIDATION_REPORT.md` and `.genius/project.json`, generated validation output.

## Hardware Evidence

Classify every hardware item as one of the following:

1. **Confirmed**: supported by an exact datasheet, board reference, schematic, or verified test.
2. **Proposed**: technically suitable but not physically verified.
3. **Blocked/TODO**: missing exact module, pinout, voltage, address, orientation, driver, power, or test evidence.

Never invent GPIO assignments, NanoKit physical pin numbers, voltages, bus addresses, limits, or electrical characteristics. Keep Proposed and Blocked/TODO hardware disabled behind Feature Flags until verification succeeds.

For every Confirmed component, document:

- Exact module name and revision.
- Purpose, signal direction, protocol, supply voltage, and logic voltage.
- NanoKit physical pin, ESP32 GPIO, and module pin.
- I2C address or multiplexer channel, SPI CS, UART baud, PWM limits, or equivalent bus data.
- Pull-ups, level shifting, protection, common ground, and power source.
- Estimated current, mounting position, axis orientation, driver status, and Feature Flag.
- Evidence source such as a datasheet, schematic, product link, or measured test.

## Required Engineering Content

Document the bill of materials, complete pin map, wiring order, power distribution, communication buses, initialization sequence, normal algorithm, failure behavior, bring-up tests, safety checks, and troubleshooting steps that apply to the example.

Mermaid diagrams should cover the physical connection and firmware flow. Add system, power, bus, boot, or safety diagrams when the example is complex enough to require them.

## Genius Guides

`.genius/` is written by a real generator: `python tools/genius/generate.py` reads the example `nanokit.yaml`, validates every pin against `registry/nanokit-esp32.yaml` (flash pins, input-only pins, strapping pins, UART0, ADC2 with Wi-Fi, 5 V inputs, double assignment) and against the firmware constants, regenerates `images/connection-diagram.md`, `images/system-diagram.md`, and the marked pin tables in `README.md` and `docs/Wiring.md`, and records the generator, command, inputs, generation time, findings, and code evidence in `.genius/VALIDATION_REPORT.md` and `.genius/project.json`. Write "Pin 36 / GPIO5" in full everywhere; never abbreviate the NanoKit pin.

## Repository Registration

After explicitly creating, removing, or renaming an example, run:

```powershell
python tools/genius/generate.py
python tools/update_repository_index.py
```

Do not run repository registration for ordinary edits that do not change the project list.

## Validation

Before handoff, build the example, run available tests and documentation checks, inspect `git diff`, and use `git diff --name-only` to verify scope. Report modified files and Confirmed, Proposed, and Blocked/TODO hardware separately. Never commit or push unless explicitly requested.
