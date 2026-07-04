# Contributing to NanoKit-ESP32

Thank you for helping improve NanoKit-ESP32.

## Contribution Areas

- Add focused examples under `examples_on_platformio/`.
- Add complete applications under `applications_on_platformio/`.
- Share community work under `open_source_projects/`.
- Add academic workspaces under `graduation_projects/`.
- Improve docs, tutorials, articles, templates, simulator assets, and shared media.

## Project Standards

- Keep PlatformIO project files scoped to their example or application folder.
- Use PlatformIO, Arduino framework for ESP32, C++, and Markdown for official examples.
- Do not use UMT SDK code or Virtual Register API code in PlatformIO examples.
- Prefer clear README files for every example, application, and template.
- Document NanoKit pin number, ESP32 GPIO number, function, reason for selection, electrical notes, and warnings whenever GPIOs are used.
- Use `docs/hardware/nanokit-integrated-esp32-pinout.md` as the pinout reference.
- Keep generated guide content inside each project `.genius/` folder.
- Do not commit local build outputs, generated firmware binaries, secrets, or editor caches.

## Example Documentation Checklist

When relevant, examples should include:

- `README.md`
- `platformio.ini`
- `src/main.cpp`
- `Wiring.md`
- Algorithm notes
- Flowchart or flowchart source
- Components list
- Bill of materials
- Testing guide
- Troubleshooting notes
- Learning notes

## Existing Content Rule

When the repository already contains content:

- Do not overwrite existing work without a clear reason.
- Analyze the current structure first.
- Build on top of what exists.
- Improve documentation and educational value.
- Preserve compatibility unless a change is intentionally documented.

## Pull Requests

Before opening a pull request, make sure the relevant example or application has a README, builds locally where possible, and includes only source, docs, tests, assets, or templates needed by the project.
