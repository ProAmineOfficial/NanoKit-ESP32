# Registry

**Developed by Amine Saoud ibn al-Bashir.**

Machine-readable board data used by `tools/genius/generate.py`.

- `nanokit-esp32.yaml` — NanoKit Integrated ESP32 V2020: header pin, GPIO, VR address, functions, and the electrical rules (flash pins, input-only pins, strapping pins, UART0, ADC2 with Wi-Fi) that every project `nanokit.yaml` is validated against.

Numbers come from `assets/images/nanokit-integrated-esp32-pinout-v2020-smd-tht.jpg`; the human-readable copy is `docs/hardware/nanokit-integrated-esp32-pinout.md`. Change the registry first, then the document, and run the generator so every project is re-validated.
