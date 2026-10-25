# Example Template

**Developed by Amine Saoud ibn al-Bashir.**

Template for new focused PlatformIO examples. Copy `examples_on_platformio/ultrasonic_distance/` and replace its content; the layout below is what the generator and the catalogs expect.

```text
example_name/
├── README.md            # keep the GENIUS:PIN_MAP markers under "## NanoKit Pin Mapping"
├── nanokit.yaml         # describe the hardware here (schema: tools/genius/README.md)
├── platformio.ini
├── src/main.cpp         # name pin constants exactly as the manifest `symbol:` fields
├── docs/Wiring.md       # keep the GENIUS:CONNECTION_TABLE markers under "## Connection Table"
├── docs/Implementation_Guide.md
├── images/              # connection-diagram.md and system-diagram.md are generated
├── .genius/             # generated validation report
├── include/ lib/ test/ assets/   # README.md in each until real content exists
```

Minimal `nanokit.yaml`:

```yaml
genius: 1
name: Example Name
category: example
status: complete
board: nanokit-esp32
summary: One sentence.
components:
  - id: led
    name: External LED
    kind: display
    status: confirmed
    power: { rail: none }
    pins:
      - { pin: Anode, gpio: 2, dir: out, symbol: LED_PIN, note: "Onboard LED pin, safe first output" }
      - { pin: Cathode, rail: GND }
```

Then run `python tools/genius/generate.py --project examples_on_platformio/example_name` and `python tools/update_repository_index.py`.
