# Software

Software architecture and tooling documentation for NanoKit-ESP32.

## Standards

| Document | Purpose |
|---|---|
| [PlatformIO Example Standard](platformio-example-standard.md) | Required structure, documentation, GPIO mapping, code quality, and Definition of Done for every official PlatformIO example. |

## Firmware Baseline

Official examples use:

- PlatformIO
- Arduino framework for ESP32
- C++
- Markdown documentation
- Normal ESP32 GPIO numbers in source code

Do not use UMT SDK code or Virtual Register API code in official PlatformIO examples.
