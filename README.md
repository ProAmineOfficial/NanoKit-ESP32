# NanoKit-ESP32

NanoKit Integrated ESP32 Dev Board is an open-source embedded development platform for IoT, AI, robotics, and UMT-based education. This repository is organized for PlatformIO examples, complete applications, community projects, graduation projects, simulation resources, documentation, articles, tutorials, and reusable templates.

## Repository Layout

| Path | Purpose |
|---|---|
| `examples_on_platformio/` | Focused PlatformIO examples for individual sensors, actuators, and board features. |
| `applications_on_platformio/` | Complete multi-part applications with ESP32 firmware, docs, assets, and generated guide folders. |
| `open_source_projects/` | Community, contributor, showcase, hackathon, and reusable open-source project areas. |
| `graduation_projects/` | Bachelor, master, engineering, thesis, institute, and capstone project workspaces. |
| `simulator/` | Web simulator, simulation engine, examples, and simulator assets. |
| `docs/` | Getting started, installation, hardware, software, API, troubleshooting, roadmap, and architecture docs. |
| `articles/` | Technical articles for embedded systems, IoT, AI, robotics, PlatformIO, NanoKit, and the UMT Platform. |
| `tutorials/` | Beginner through advanced tutorials, project guides, and video link indexes. |
| `templates/` | Reusable templates for examples, applications, thesis work, and documentation. |
| `assets/` | Shared images, icons, logos, banners, and videos. |

## PlatformIO Baseline

PlatformIO projects use ESP32 + Arduino framework settings as the first public baseline. Dedicated NanoKit board definitions can replace the generic ESP32 board profile as the hardware package evolves.

## UMT Development Rule

UMT-facing examples and documentation should use Virtual Register naming and the UMT API model where applicable. Low-level hardware details belong in board profiles, backend notes, generated outputs, or implementation documentation.
