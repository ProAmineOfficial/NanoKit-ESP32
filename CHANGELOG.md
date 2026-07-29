# Changelog

All notable changes to NanoKit-ESP32 will be documented in this file.

## Unreleased

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
