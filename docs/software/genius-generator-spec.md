# Genius Generator Specification

**Developed by Amine Saoud ibn al-Bashir.**

Design record for the automatic hardware-documentation system of NanoKit-ESP32. Approved 2026-10-25. Implementation: `tools/genius/generate.py`; user guide: `tools/genius/README.md`.

## 1. Problem

Before this system every project described its hardware three times by hand: in the firmware constants, in the README and Wiring tables, and in Mermaid diagrams. The three drifted. The audit of 2026-10-25 found:

- The drone diagrams named only GPIO numbers, never NanoKit header pins, against the repository rule `NanoKit Pin -> GPIO -> Component Pin`.
- The drone system diagram showed one sensor while nine planned sensors existed only in `feature_flags.h` and prose.
- `docs/hardware/nanokit-integrated-esp32-pinout.md` carried four wrong Virtual Register addresses for the ADC pins (0x5E/0x5B/0x58/0x5C instead of 0x56/0x53/0x50/0x54 on the datasheet).
- The repository index counted category folders as projects and could not see firmware kept under `esp32/` or `firmware/`.
- Nothing stopped a contributor from assigning a flash pin, driving an input-only pin, or feeding 5 V into a GPIO.

## 2. Decision

**Hybrid source of truth.** A per-project manifest `nanokit.yaml` is the authoritative description of the hardware; the firmware constants are evidence that must agree with it; a generator validates both against a machine-readable board registry and produces every pin table and wiring diagram. Hand-drawn diagrams remain only for algorithms (flowcharts, control loops, motor layout).

Alternatives considered: (A) parse firmware comments only — no semantics (bus, voltage, status), fragile; (B) manifest only — drifts silently from code. The hybrid keeps the manifest readable and makes drift a CI failure.

## 3. Components

| Unit | Purpose | Interface | Depends on |
|---|---|---|---|
| `registry/nanokit-esp32.yaml` | NanoKit board as data: header pin, GPIO, VR address, functions, rules | YAML read by the generator and by humans | datasheet image |
| `<project>/nanokit.yaml` | Project hardware as data | schema 1, documented in `tools/genius/README.md` | registry ids |
| `tools/genius/generate.py` | validate, cross-check evidence, render, report | CLI (`--check`, `--project`, `--render`, `--json`); exit 1 on error | PyYAML |
| `tools/genius/tests/` | rule self-tests on a throw-away project | pytest | generator |
| `tools/update_repository_index.py` | catalogs and indexes from manifests and `.genius/project.json` | CLI | stdlib only |
| `.githooks/pre-commit`, `.github/workflows/repository-index.yml` | run both tools; PRs are check-only, pushes to `main` regenerate and commit | git | Python 3 |

## 4. Data flow

1. Discover every `nanokit.yaml` under the four project areas.
2. Schema validation (ids, kinds, statuses, categories match folders, unknown pin keys).
3. Hardware validation against the registry (flash, input-only, UART0, strapping, ADC2/Wi-Fi, DAC, 5 V inputs, double assignment, header mismatch, I2C collisions, confirmed-needs-gpio).
4. Evidence: scan `code_roots` for `constexpr`/`const`/`#define` scalars and `std::array`/C-array initialisers; compare each `symbol` with the manifest GPIO.
5. Render `images/connection-diagram.md` and `images/system-diagram.md`; replace the marked blocks in `README.md` and `docs/Wiring.md`; write `.genius/VALIDATION_REPORT.md`, `.genius/project.json`, `.genius/README.md`.
6. Exit 1 if any project has an error.

## 5. Status model

`confirmed` (solid), `proposed` (dashed yellow), `blocked` (dashed red). Blocked components keep `gpio: null` and still appear in both diagrams, so planned hardware is visible without claiming a pin. A confirmed pin without a GPIO is an error; a blocked pin with a GPIO is a warning.

## 6. Error handling

- Validation findings are collected, never raised; the report lists all of them with rule ids.
- Missing markers in README/Wiring produce a warning and leave the file untouched.
- Missing PyYAML exits with code 2 and an install hint.
- SVG rendering is optional and never fails the run.

## 7. Testing

`tools/genius/tests/test_generate.py` builds a temporary project inside the repository, feeds it each forbidden configuration, and asserts the rule fires; it also checks the registry against the datasheet values and that generated Mermaid carries `Pin <NanoKit pin> / GPIO<n>` labels. CI runs the tests and `--check` on every pull request.

## 8. Out of scope

Flowchart generation, PlatformIO builds in CI (the toolchain download is large; add later as a separate job), and physical bench verification, which the `status` field records as a human statement.
