# PlatformIO Example Standard

This document turns the master project specification into a practical checklist for every NanoKit-ESP32 example.

Official examples must compile with PlatformIO using the ESP32 Arduino framework and normal ESP32 GPIO numbers.

## Required Directory Layout

Every example should use this structure:

```text
example_name/
├── README.md
├── platformio.ini
├── src/
│   └── main.cpp
├── include/
│   └── README.md
├── lib/
│   └── README.md
├── test/
│   └── README.md
├── assets/
│   └── README.md
├── images/
│   └── README.md
└── docs/
    ├── Wiring.md
    ├── Algorithm.md
    ├── Flowchart.md
    ├── Pseudocode.md
    ├── Components.md
    ├── Testing.md
    ├── Troubleshooting.md
    ├── Learning_Notes.md
    └── Exercises.md
```

Folders should not be left empty. If a folder has no source code yet, include a README explaining what belongs there and why the folder exists.

## Required `platformio.ini`

Use this baseline unless the example needs extra libraries or build flags:

```ini
[env:nanokit_esp32]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600
```

When libraries are needed, list them under `lib_deps` with a short explanation in the example README.

## Required README Sections

Every example README should include:

- Goal
- What You Will Learn
- Hardware Required
- NanoKit Pin Mapping
- Wiring Summary
- How It Works
- Build and Upload
- Serial Monitor Output, when relevant
- Troubleshooting
- Exercises
- Suggested Improvements
- Related Documentation

## Hardware Documentation Rules

Every GPIO used in an example must be documented using this table:

| NanoKit Pin | GPIO | Component Pin | Direction | Reason | Electrical Notes |
|---:|---|---|---|---|---|
| 17 | GPIO2 | Onboard LED | Output | Safe first digital output and PWM-capable pin | 3.3 V logic |

Use `docs/hardware/nanokit-integrated-esp32-pinout.md` as the source of truth.

## Wiring Document

Every hardware example that uses board pins must include `docs/Wiring.md`.

The wiring table must use this mapping:

```text
NanoKit Pin -> GPIO -> Electronic Component Pin -> Description
```

Example:

| NanoKit Pin | GPIO | Component Pin | Description |
|---:|---|---|---|
| 23 | GPIO21 | OLED SDA | I2C data line |
| 18 | GPIO22 | OLED SCL | I2C clock line |
| 12 or 31 | GND | OLED GND | Common ground |
| 13 or 14 | 3V3 | OLED VCC | 3.3 V power |

## Code Requirements

Source code should be educational and maintainable.

- Use meaningful constants for GPIO pins.
- Explain why each GPIO was selected.
- Avoid magic numbers in logic.
- Keep `setup()` and `loop()` readable.
- Extract helper functions when it improves clarity.
- Comment important lines with intent, not just syntax.
- Avoid unsafe pins listed in the hardware reference.

Example:

```cpp
const int LED_PIN = 2;  // NanoKit Pin 17, GPIO2, onboard LED and PWM-capable safe output.

void setup() {
  pinMode(LED_PIN, OUTPUT);  // Configure GPIO2 as an output so the ESP32 can drive the onboard LED.
}
```

## Required Documentation Files

Use these files when relevant:

| File | Purpose |
|---|---|
| `docs/Wiring.md` | Physical wiring and pin mapping. |
| `docs/Algorithm.md` | Step-by-step behavior of the example. |
| `docs/Flowchart.md` | Mermaid or image-based flowchart. |
| `docs/Pseudocode.md` | Language-neutral logic explanation. |
| `docs/Components.md` | Hardware list and bill of materials. |
| `docs/Testing.md` | Build, upload, run, and verification steps. |
| `docs/Troubleshooting.md` | Common failures and fixes. |
| `docs/Learning_Notes.md` | Theory and embedded-systems concepts. |
| `docs/Exercises.md` | Practice tasks and suggested improvements. |

## Diagram Policy

Prefer diagrams that are useful for learning:

- Connection diagram
- Breadboard diagram
- Block diagram
- System architecture diagram
- Flowchart

If a generated image is not available yet, include a Mermaid diagram or a clear Markdown diagram source that can be rendered later.

## Definition of Done

An example is complete only when:

- `platformio.ini` exists.
- `src/main.cpp` exists.
- Source code is educational and maintainable.
- GPIO choices are explained.
- Pin mapping is documented.
- Wiring is documented when hardware is used.
- Algorithm, flowchart, and pseudocode are present when useful.
- Components and testing notes exist.
- Troubleshooting notes exist.
- Exercises or suggested improvements exist.
- The example can be built with PlatformIO.

## Build Command

From the example directory:

```bash
pio run
```

If PlatformIO is not installed, install it through the official PlatformIO extension or CLI before building.
