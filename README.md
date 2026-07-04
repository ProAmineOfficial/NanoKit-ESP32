# NanoKit-ESP32

NanoKit Integrated ESP32 Dev Board is an open-source embedded development platform for IoT, AI, robotics, education, and professional prototyping. This repository is organized for PlatformIO examples, complete applications, community projects, graduation projects, simulation resources, documentation, articles, tutorials, and reusable templates.

The NanoKit board is part of the wider UMT hardware vision, but this repository's firmware examples use standard PlatformIO Arduino C++ and ESP32 GPIO numbers so they can compile and run directly in PlatformIO.

## Repository Mission

This is not a demo repository and not a template repository. It is the official long-term NanoKit-ESP32 ecosystem repository for educational and professional embedded systems development.

See [PROJECT_SPECIFICATION.md](PROJECT_SPECIFICATION.md) for the master repository rules, growth model, example standards, documentation requirements, and Definition of Done.

## Quality Contract

Official content in this repository should be production-ready, educational, complete, and maintainable.

- Do not leave directories empty as final work.
- Do not add placeholder content when real educational material can be provided.
- Build on existing work instead of overwriting it unnecessarily.
- Every example should teach the code, wiring, algorithm, pin mapping, and troubleshooting path.
- Every hardware example must document NanoKit pin numbers and ESP32 GPIO numbers.

Use [docs/software/platformio-example-standard.md](docs/software/platformio-example-standard.md) when creating or reviewing examples.

## Repository Layout

| Path | Purpose |
|---|---|
| `examples_on_platformio/` | Focused PlatformIO examples for individual sensors, actuators, and board features. |
| `applications_on_platformio/` | Complete multi-part applications with ESP32 firmware, docs, assets, and generated guide folders. |
| `open_source_projects/` | Community, contributor, showcase, hackathon, and reusable open-source project areas. |
| `graduation_projects/` | Bachelor, master, engineering, thesis, institute, and capstone project workspaces. |
| `simulator/` | Web simulator, simulation engine, examples, and simulator assets. |
| `docs/` | Getting started, installation, hardware, software, API, troubleshooting, roadmap, and architecture docs. |
| `articles/` | Technical articles for embedded systems, IoT, AI, robotics, PlatformIO, NanoKit, and related ecosystem topics. |
| `tutorials/` | Beginner through advanced tutorials, project guides, and video link indexes. |
| `templates/` | Reusable templates for examples, applications, thesis work, and documentation. |
| `assets/` | Shared images, icons, logos, banners, and videos. |

## PlatformIO Baseline

PlatformIO projects use the ESP32 Arduino framework as the first public baseline:

```ini
[env:nanokit_esp32]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600
```

Dedicated NanoKit board definitions can replace the generic `esp32dev` board profile as the hardware package evolves.

## Firmware Rule

Official examples in this repository must use:

- PlatformIO
- Arduino framework for ESP32
- C++
- Markdown documentation
- Normal ESP32 GPIO numbers in source code

Do not use UMT SDK code or Virtual Register API code in PlatformIO examples.

## Hardware Pinout

Use [docs/hardware/nanokit-integrated-esp32-pinout.md](docs/hardware/nanokit-integrated-esp32-pinout.md) as the source of truth for NanoKit pin numbers, ESP32 GPIO mappings, electrical notes, and warnings.

Every hardware example should document:

```text
NanoKit Pin -> GPIO -> Component Pin -> Description
```

Important restrictions:

- GPIO34, GPIO35, GPIO36, and GPIO39 are input-only.
- GPIO6, GPIO7, GPIO8, GPIO9, GPIO10, and GPIO11 are internal SPI flash pins and must not be used.
- GPIO0 is the BOOT pin.
- GPIO1 and GPIO3 are shared with USB Serial.
- EN is Reset / Enable and is not a GPIO.
