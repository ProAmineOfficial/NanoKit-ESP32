# NanoKit-ESP32 Project Specification

## Mission

NanoKit-ESP32 is the official open-source PlatformIO repository for the NanoKit Integrated ESP32 development board.

This is not a demo repository. It is a long-term educational and professional embedded systems platform inspired by the clarity of Arduino examples and the structure of PlatformIO projects, redesigned specifically for NanoKit Integrated ESP32.

The repository should continuously grow over time. Every commit should make the ecosystem richer, more useful, and more complete.

## Audience

This repository serves:

- Beginners
- Students
- Universities
- Embedded engineers
- Researchers
- Open-source contributors
- Industrial developers

## Mandatory Technology Stack

Use only:

- PlatformIO
- Arduino framework for ESP32
- C++
- Markdown

Do not use UMT SDK code in PlatformIO examples.
Do not use Virtual Register API code in PlatformIO examples.
Do not use experimental languages for official examples.

The NanoKit hardware may be documented as part of the wider UMT product vision, but all firmware examples in this repository must compile as normal PlatformIO Arduino C++ projects using ESP32 GPIO numbers.

## Repository Philosophy

- Build a complete examples library, not a small demo set.
- Prefer meaningful educational content over placeholder folders.
- Keep examples focused, understandable, and buildable.
- Keep applications complete and realistic.
- Preserve existing work when adding new content.
- Improve documentation and code quality as the repository evolves.

## Example Library

Examples should be organized into categories such as:

- Basics
- GPIO
- Digital IO
- Analog
- PWM
- Interrupts
- Timers
- UART
- SPI
- I2C
- WiFi
- Bluetooth
- BLE
- HTTP
- MQTT
- OTA
- Filesystem
- EEPROM
- Camera
- Sensors
- Motors
- Displays
- Industrial
- AI
- Power
- Sleep
- Advanced

Every example should contain:

```text
README.md
platformio.ini
src/main.cpp
include/
lib/
assets/
docs/
images/
```

Empty folders should be avoided when useful content can be provided. If a folder must be reserved, include a short README explaining its future purpose.

## Required Example Documentation

Generate only the documents that are relevant to the example, but prefer rich documentation. Useful documents include:

- Hardware wiring
- Algorithm
- Flowchart
- Pseudocode
- Components list
- Bill of materials
- Testing guide
- Troubleshooting
- Learning notes
- PDF report source or export instructions

Each example that uses external wiring should include `Wiring.md` with this mapping style:

```text
NanoKit Pin -> GPIO -> Component Pin -> Description
```

## Code Style

Every example should be educational. Source code should explain what the code does and why each hardware choice exists.

Example:

```cpp
pinMode(2, OUTPUT);   // Configure GPIO2 as an output because NanoKit Pin 17 is connected to the onboard LED.
```

Comments must be meaningful. Avoid noisy comments that only repeat syntax.

## Hardware Documentation Rules

Whenever an example uses GPIOs, document:

- NanoKit pin number
- ESP32 GPIO number
- Function
- Reason for selecting it
- Electrical notes
- Warnings and restrictions

Use `docs/hardware/nanokit-integrated-esp32-pinout.md` as the primary pinout reference.

## Open Source Projects

Open-source projects should be original NanoKit implementations inspired by useful public ideas. Do not copy repositories. Study ideas, redesign them, improve them, adapt them to NanoKit, and document the differences.

Suggested domains:

- IoT
- Robotics
- AI
- Agriculture
- Healthcare
- Industry
- Energy
- Home automation
- Security
- Education

## Graduation Projects

Graduation projects should be complete academic workspaces. Each project should include:

- README
- PlatformIO project
- Source code
- Documentation
- Architecture
- Flowchart
- Algorithm
- Bill of materials
- Images
- PDF report source or export instructions
- Presentation outline
- Future improvements

## Tutorials

Tutorials should progress from beginner to advanced and should explain:

- Theory
- Code
- Wiring
- Common mistakes
- Improvements
- Exercises

## Existing Content Rule

When the repository already contains content:

- Do not overwrite existing work without a clear reason.
- Analyze the current repository first.
- Continue building on top of it.
- Add new examples.
- Improve existing examples.
- Expand documentation.
- Increase educational value.
- Refactor code when necessary while preserving compatibility.

Treat this repository as a living NanoKit ecosystem.
