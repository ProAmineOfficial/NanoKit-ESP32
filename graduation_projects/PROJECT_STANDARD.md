# Graduation Project Engineering Standard

**Developed by Amine Saoud ibn al-Bashir.**

This standard applies to bachelor, master, engineering, institute, capstone, and university-thesis projects inside `graduation_projects/`. It defines a reproducible academic project package; it is not a background automation service.

## Academic Project Package

Each implemented graduation project should normally include:

- `README.md` with the problem statement, objectives, scope, requirements, architecture, and reproduction steps.
- Literature and design references with clear source attribution.
- Project-owned firmware, applications, internal libraries, configuration, datasets, and tests as applicable.
- Hardware inventory, bill of materials, cost assumptions, and procurement references.
- Complete wiring, pin mapping, power, grounding, protection, and bus documentation.
- Implementation guide with algorithms, pseudocode, flowcharts, initialization, and failure behavior.
- Experimental method, calibration procedure, test plan, acceptance criteria, results, limitations, and future work.
- Mermaid system, connection, power, communication, firmware-flow, boot, and safety diagrams as applicable.
- User, installation, build, upload, filesystem-upload, and Serial Monitor instructions where relevant.
- A `nanokit.yaml` hardware manifest and the `.genius/` reports it generates.

Use `examples_on_platformio/ultrasonic_distance/` as the minimum detail benchmark for hardware-facing documentation, while expanding the academic material to fit the thesis or capstone objectives.

## Hardware Evidence

Classify every component as **Confirmed**, **Proposed**, or **Blocked/TODO**. Never invent NanoKit pins, ESP32 GPIOs, module revisions, voltages, addresses, electrical limits, or measured performance.

For every Confirmed component, document:

- Exact module name, revision, quantity, purpose, and evidence source.
- Module pin, direction, protocol, voltage, and logic level.
- NanoKit physical pin and ESP32 GPIO.
- Bus address or channel, SPI CS, UART baud, PWM limits, or equivalent configuration.
- Pull-ups, level shifting, protection, common ground, power source, and estimated current.
- Mounting location, axis orientation, driver, Feature Flag, calibration, and bring-up result.

Proposed and Blocked/TODO hardware must state the missing evidence and remain disabled until verification and testing succeed.

## Reproducibility and Safety

Document software versions, build environment, test equipment, calibration data, experiment conditions, expected outputs, failure handling, and safety constraints. Separate observed results from assumptions and future proposals.

## Genius Guides

`.genius/` is written by a real generator: `python tools/genius/generate.py` reads the project `nanokit.yaml`, validates every pin against `registry/nanokit-esp32.yaml` and the firmware constants, regenerates `images/connection-diagram.md`, `images/system-diagram.md`, and the marked pin tables, and records the generator, command, inputs, generation time, findings, and code evidence in `.genius/VALIDATION_REPORT.md` and `.genius/project.json`. Classify every component in the manifest as `confirmed`, `proposed`, or `blocked`; blocked components keep `gpio: null` and still appear in the diagrams.

## Repository Registration

Run `python tools/update_repository_index.py` only after explicitly creating, removing, or renaming a graduation project. Ordinary edits do not require repository registration.

## Validation

Build affected code, run available tests and documentation checks, inspect `git diff`, and verify scope with `git diff --name-only`. Report modified files, validation evidence, and Confirmed, Proposed, and Blocked/TODO hardware separately. Never commit or push unless explicitly requested.
