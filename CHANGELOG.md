# Changelog

**Developed by Amine Saoud ibn al-Bashir.**

All notable changes to NanoKit-ESP32 will be documented in this file.

## Unreleased

- Added the Genius hardware generator (`tools/genius/generate.py`): every project now describes its hardware once in `nanokit.yaml`; the generator validates it against the new board registry `registry/nanokit-esp32.yaml` (flash pins, input-only pins, strapping pins, UART0, ADC2 with Wi-Fi, 5 V inputs, double assignment, I2C collisions) and against the firmware pin constants, then regenerates `images/connection-diagram.md`, `images/system-diagram.md`, the README pin tables, the `docs/Wiring.md` connection tables, and `.genius/` validation reports. Self-tests live in `tools/genius/tests/`.
- Added `nanokit.yaml` manifests for ultrasonic distance, stepper motor control, camera tracking, face rotation tracking camera, and NanoKit Drone 4X (two firmware nodes, three buses, 20 components with Confirmed / Proposed / Blocked status).
- Fixed four Virtual Register addresses in the pinout reference: VR_ADC_IN6/IN3/IN0/IN4 are 0x56/0x53/0x50/0x54 as printed on the board datasheet (previously 0x5E/0x5B/0x58/0x5C).
- Fixed the repository index: a folder is a project only when it carries `nanokit.yaml`, firmware under `esp32/` or `firmware/` is detected, category folders are listed separately, and a Genius validation column was added; the graduation catalog is generated too.
- NanoKit Drone 4X: removed 492 auto-inserted filler comments (one third of the source lines) and replaced the pin constant comments with NanoKit header pin, GPIO, and VR address; documented all planned sensors, the payload node, the power chain, and the Flight Deck in generated diagrams with NanoKit pin numbers (Pin 3/Pin 37/Pin 19/Pin 7 for M1-M4, Pin 23/Pin 18 for I2C0).
- Removed an accidentally committed `less` help screen file from the repository root.
- Added `.gitattributes` so `.githooks/*` and `*.sh` always keep LF line endings; on Windows checkouts with `core.autocrlf=true` a CRLF hook cannot run.
- Generated files are now byte-stable: the generator rewrites a file only when its content changes beyond the timestamp and command line, so the hook and the workflow no longer create a commit on every run.
- Aligned the PlatformIO Example Standard, CONTRIBUTING, the three PROJECT_STANDARD files, and the example template with the real example layout (`docs/Wiring.md` + `docs/Implementation_Guide.md`) and the manifest workflow.
- The GitHub workflow now validates manifests and runs the generator self-tests on pull requests, and regenerates diagrams, tables, and indexes on pushes to `main`; the pre-commit hook does the same locally.
- Added generated repository index and tree tooling with a local pre-commit hook and GitHub Actions verification.
- Added NanoKit Drone 4X (Quadcopter), an open-source Quad-X reference project with ESP32 BLE firmware, browser controller, MPU6050 attitude estimation, four brushless ESC outputs, calibration, PID tuning, motor/wiring diagrams, test gates, and safety documentation.
- Fixed NanoKit Drone 4X PlatformIO C++ compatibility diagnostics, added developer attribution for Amine Saud Ibn Bashir, redesigned the controller as a camera-centred Flight Deck, and documented the separate Arducam Mega 5 MP Wi-Fi camera node.
- Replaced scaffold-only PlatformIO `src/.gitkeep` folders with complete Arduino C++ `src/main.cpp` implementations for ultrasonic distance, stepper motor control, camera tracking, and face rotation tracking firmware.
- Added wiring guides, implementation guides, Mermaid diagram sources, richer project READMEs, and explanatory README files for include/lib/test/assets folders in the converted PlatformIO projects.
- Replaced the project specification with the master NanoKit-ESP32 specification covering repository philosophy, required PlatformIO stack, example library scope, code quality, hardware documentation, applications, open-source projects, graduation projects, tutorials, articles, documentation, and Definition of Done.
- Added the PlatformIO Example Standard with required example structure, README sections, wiring rules, code requirements, documentation files, diagram policy, and build checklist.
- Updated README, software docs, and contribution rules to expose the master specification and no-placeholder quality contract.
- Added the NanoKit Integrated ESP32 pinout reference with GPIO mappings, board pin numbers, wiring template, recommended pins, and hardware restrictions.
- Scaffolded the public repository structure for examples, applications, open-source projects, graduation projects, simulator resources, docs, articles, tutorials, templates, and shared assets.
- Added contribution guidelines and a PlatformIO-focused `.gitignore`.
