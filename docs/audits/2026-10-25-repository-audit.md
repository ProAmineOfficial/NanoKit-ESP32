# Repository Audit - 2026-10-25

**Developed by Amine Saoud ibn al-Bashir.**

Scope: the `main` snapshot of `ProAmineOfficial/NanoKit-ESP32` (327 files) and the Git Architecture Diagram export of commit `5b30d5c` (structural analysis, Deep Genius not run). Method: every Mermaid block rendered with `mmdc`, every pin constant compared with `assets/images/nanokit-integrated-esp32-pinout-v2020-smd-tht.jpg`, every generated catalog compared with the folders on disk.

## What was already right

- All 16 Mermaid diagrams rendered without syntax errors.
- Every GPIO in the three examples and the application mapped to the correct NanoKit header pin (GPIO5 = Pin 36, GPIO34 = Pin 2, GPIO25 = Pin 3, GPIO26 = Pin 37, GPIO27 = Pin 19, GPIO32 = Pin 7, GPIO2 = Pin 17, GPIO21 = Pin 23, GPIO22 = Pin 18).
- `examples_on_platformio/ultrasonic_distance/` was a sound reference: clean code, educational comments, correct pins, complete documentation.

## Findings and fixes

| # | Location | Finding | Fix |
|---|---|---|---|
| 1 | `docs/hardware/nanokit-integrated-esp32-pinout.md` | Four Virtual Register addresses were offset by +8: Pin 2 0x5E, Pin 4 0x5B, Pin 5 0x58, Pin 7 0x5C. The datasheet prints 0x56, 0x53, 0x50, 0x54. | Corrected; the registry carries the datasheet values and a self-test pins them. |
| 2 | repository root | A 17 KB file named `no --cached --check option  (press RETURN)` (the `less` help screen) had been committed. | Deleted. |
| 3 | `tools/update_repository_index.py` | Only `firmware/platformio.ini` was detected, so the application with `esp32/platformio.ini` showed "Firmware: no"; category folders (`templates/`, `hackathon_projects/`, ...) were counted as projects (open_source 6, graduation 7; real: 1 and 0). | A project is a folder with `nanokit.yaml`; firmware paths come from the manifest; categories are listed separately; a Genius validation column was added. |
| 4 | NanoKit Drone 4X sources | 492 filler comments such as `// Close the current scope or type definition.` made up one third of 1 426 lines, against `PROJECT_SPECIFICATION §2`. | Removed (code verified identical after stripping comments in all 28 files); pin constants now carry `NanoKit Pin 23, GPIO21, VR 0x30` style comments. |
| 5 | NanoKit Drone 4X docs and diagrams | No NanoKit header pin anywhere, GPIO only, against the `NanoKit Pin -> GPIO -> Component Pin` rule. | Generated tables and diagrams label `Pin 3 / GPIO25` etc.; motor layout table gained NanoKit pin and LEDC channel columns. |
| 6 | `drone/images/system-diagram.md` | Only ICM-20948 was drawn; nine planned sensors existed only in `feature_flags.h` and prose. | The manifest declares 20 components and 3 buses with Confirmed / Proposed / Blocked status; the generated system diagram shows all of them, grouped by node and bus. |
| 7 | `docs/software/platformio-example-standard.md`, `CONTRIBUTING.md` | Demanded nine `docs/*.md` files and `images/README.md`; real examples use `Wiring.md` + `Implementation_Guide.md` + `images/connection-diagram.md`. | Standard and checklist now describe the real layout and the manifest workflow. |
| 8 | Four project areas | Firmware lived at `src/`, `esp32/src/`, `firmware/src/` + `camera_node/`; only the drone had a manifest; every `.genius/README.md` said "no generator installed". | One manifest schema with a `firmware:` list covers all layouts; `.genius/` is now written by a real generator. |
| 9 | `drone/firmware/web_control/assets/nanokit-drone-4x-reference.png` | Looks like a commercial product photograph. | Not changed; verify the licence or replace it before relying on it in an open-source repository. |

## What the generator adds

- `registry/nanokit-esp32.yaml`: the 40-pin header as data, including the rules (flash pins GPIO6-11, input-only GPIO34/35/36/39, strapping GPIO0/2/12/15, UART0 GPIO1/3, ADC2 with Wi-Fi).
- `tools/genius/generate.py`: validation, code-evidence cross-check, diagram and table generation, `.genius/` reports; exit 1 on error so the pre-commit hook and the pull-request workflow block mistakes. Demonstrated: assigning GPIO6 or changing a manifest pin without changing the code fails with `flash-pin` and `evidence` errors.
- `tools/genius/tests/`: 13 self-tests covering every rule.
- Manifests for the five projects, markers in their README and Wiring files, regenerated diagrams and tables.

## Not verified here

PlatformIO builds were not run in this environment (the toolchain download is blocked). The only firmware change is comment text, verified by comparing comment-stripped sources before and after.
