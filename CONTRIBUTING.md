# Contributing to NanoKit-ESP32

Thank you for helping improve NanoKit-ESP32.

This repository is the official educational and professional PlatformIO ecosystem for NanoKit Integrated ESP32. Contributions should be production-ready, educational, complete, and maintainable.

## Contribution Areas

- Add focused examples under `examples_on_platformio/`.
- Add complete applications under `applications_on_platformio/`.
- Share original NanoKit-oriented work under `open_source_projects/`.
- Add academic workspaces under `graduation_projects/`.
- Improve docs, tutorials, articles, templates, simulator assets, and shared media.

## Project Standards

- Keep PlatformIO project files scoped to their example or application folder.
- Use PlatformIO, Arduino framework for ESP32, C++, and Markdown for official examples.
- Do not use UMT SDK code or Virtual Register API code in PlatformIO examples.
- Do not add placeholder content as final work.
- Do not leave directories empty when real educational material can be provided.
- Prefer clear README files for every example, application, and template.
- Document NanoKit pin number, ESP32 GPIO number, function, reason for selection, electrical notes, and warnings whenever GPIOs are used.
- Use `docs/hardware/nanokit-integrated-esp32-pinout.md` as the pinout reference.
- Use `docs/software/platformio-example-standard.md` as the example structure and Definition of Done reference.
- Keep generated guide content inside each project `.genius/` folder.
- Do not commit local build outputs, generated firmware binaries, secrets, or editor caches.

## Example Documentation Checklist

When relevant, examples should include:

- `README.md`
- `platformio.ini`
- `src/main.cpp`
- `docs/Wiring.md`
- `docs/Algorithm.md`
- `docs/Flowchart.md`
- `docs/Pseudocode.md`
- `docs/Components.md`
- `docs/Testing.md`
- `docs/Troubleshooting.md`
- `docs/Learning_Notes.md`
- `docs/Exercises.md`
- Assets or diagrams when useful
- PDF-ready documentation structure

## Definition of Done

A contribution is complete only when:

- Code works or the documentation-only scope is explicit.
- PlatformIO builds successfully for firmware examples.
- Documentation exists.
- Wiring exists where hardware is used.
- Pin mapping exists where hardware is used.
- Algorithm, flowchart, and pseudocode exist where useful.
- Testing instructions exist.
- Troubleshooting notes exist.
- Learning notes or exercises exist for educational examples.
- A beginner can understand the project without external resources.

## Existing Content Rule

When the repository already contains content:

- Do not overwrite existing work without a clear reason.
- Analyze the current structure first.
- Build on top of what exists.
- Improve documentation and educational value.
- Preserve compatibility unless a change is intentionally documented.

## Pull Requests

Before opening a pull request, make sure the relevant example or application has a README, builds locally where possible, and includes only source, docs, tests, assets, or templates needed by the project.
