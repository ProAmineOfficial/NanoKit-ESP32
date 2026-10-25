# NanoKit Genius Generator

**Developed by Amine Saoud ibn al-Bashir.**

`generate.py` turns one small file per project, `nanokit.yaml`, into validated hardware documentation. It is the automation that the `.genius/` directories reserve space for.

```text
registry/nanokit-esp32.yaml   the board as data (header pin <-> GPIO <-> VR address <-> rules)
<project>/nanokit.yaml        the project hardware as data (components, buses, pins, status)
<project>/src/**              firmware constants, read as evidence
            |
            v  python tools/genius/generate.py
<project>/images/connection-diagram.md      physical wiring, labels "Pin 36 / GPIO5 · TRIG"
<project>/images/system-diagram.md          every component grouped by node and bus
<project>/README.md            (block)      NanoKit Pin Mapping table
<project>/docs/Wiring.md       (block)      Connection table with VR addresses and status
<project>/.genius/VALIDATION_REPORT.md      generator, command, inputs, rules, findings, evidence
<project>/.genius/project.json              the same for machines
```

## Run

```bash
pip install pyyaml
python tools/genius/generate.py              # validate + generate every project
python tools/genius/generate.py --check      # validate only (what CI runs on pull requests)
python tools/genius/generate.py --project examples_on_platformio/ultrasonic_distance
python tools/genius/generate.py --render     # also write SVG next to each diagram when mmdc exists
python -m pytest tools/genius/tests -q       # rule self-tests
```

Exit status 1 when any project has an error, so the pre-commit hook and the GitHub workflow stop.

## Manifest reference (`nanokit.yaml`, schema 1)

```yaml
genius: 1                       # schema version (required)
name: Ultrasonic Distance       # human name (required)
category: example               # example | application | open_source | graduation (must match the folder area)
status: complete                # complete | bench-development | in-progress | planned (free text, shown in catalogs)
board: nanokit-esp32            # registry id (required)
summary: One sentence.

firmware:                       # one entry per PlatformIO target; default is a single "main" at "."
  - id: main
    name: NanoKit flight controller      # optional label for diagrams
    role: sole motor authority           # optional, appended to the label
    path: .                              # folder that holds platformio.ini
    code_roots: [src, include]           # scanned for pin constants (relative to path)
    wifi: false                          # true enables the ADC2-with-Wi-Fi rule

docs:                           # optional; these are the defaults
  readme: README.md             # must contain the GENIUS:PIN_MAP markers
  wiring: docs/Wiring.md        # must contain the GENIUS:CONNECTION_TABLE markers
  images: images                # where the two diagrams are written

buses:                          # shared buses, declared once, devices attach by id
  - id: i2c0
    type: i2c                   # i2c | spi | uart | i2s | onewire | can
    firmware: main              # optional, defaults to the first firmware
    status: confirmed           # confirmed | proposed | blocked
    speed: 400 kHz              # optional label
    pins:
      - { role: SDA, gpio: 21, symbol: I2C_SDA_PIN, note: "..." }
      - { role: SCL, gpio: 22, symbol: I2C_SCL_PIN }

components:
  - id: hcsr04                  # unique, used in links
    name: HC-SR04 ultrasonic sensor
    kind: sensor                # sensor | actuator | driver | display | storage | comm | power | node | other
    status: confirmed           # confirmed | proposed | blocked
    firmware: main              # optional
    bus: i2c0                   # optional: attach to a declared bus
    address: "0x68"             # optional, I2C address (collisions are detected)
    power: { rail: 3V3 }        # 3V3 | 5V | GND | external | none
    note: Free text shown in the report.
    pins:
      - { pin: TRIG, gpio: 5,  dir: out, symbol: TRIGGER_PIN, note: "..." }
      - { pin: ECHO, gpio: 34, dir: in,  symbol: ECHO_PIN, level: 5V, level_shift: required }
      - { pin: CS,   gpio: null, dir: out }        # allowed only when status is proposed/blocked
      - { pin: VCC,  rail: 3V3 }
      - { pin: GND,  rail: GND }

externals:                      # actors that are not hardware on the board
  - { id: deck, name: Flight Deck browser }

links:                          # logical links drawn in both diagrams
  - { from: deck, to: main, label: "Wi-Fi WebSocket v3", kind: both }   # one | both | dashed | line
```

Pin keys: `pin`, `gpio`, `dir` (`out` | `in` | `bidir`), `type` (`gpio` default | `pwm` | `adc` | `dac` | `touch` | `uart` | `spi` | `i2s` | `i2c`), `symbol`, `note`, `level`, `level_shift` (`required` | `present` | `divider` | `shifter`), `rail`, `header` (optional cross-check of the NanoKit header number).

**YAML note:** a comma inside a flow-style `{ ... }` value starts a new key. Quote notes that contain commas; the generator reports unknown keys so the mistake cannot pass silently.

## Rules

| Rule | Level | Meaning |
|---|---|---|
| `flash-pin` | error | GPIO6-11 (Pin 21, Pin 22, Pin 27-Pin 30) are the internal SPI flash. |
| `input-only` | error | GPIO34/35/36/39 (Pin 2, Pin 6, Pin 5, Pin 4) cannot be `out`, `bidir`, or `pwm`. |
| `uart0-shared` | error | GPIO1/GPIO3 (Pin 25/Pin 26) are the USB-serial console. |
| `strapping` | error / warning / info | GPIO0 is an error; GPIO12 and GPIO15 warn; GPIO2 and GPIO5 are informational. |
| `adc2-wifi` | error / warning | `type: adc` on an ADC2 pin while `wifi: true` is an error; without Wi-Fi it warns. |
| `dac-capable`, `touch-capable`, `adc-capable` | error | The pin lacks that peripheral. |
| `5v-input` | error / warning | A 5 V signal into a GPIO needs `level_shift`; declared shifting still warns so the reader sees it. |
| `double-assigned` | error | Two signals on one GPIO within the same firmware. |
| `header-mismatch` | error | A declared `header:` does not carry that GPIO on NanoKit. |
| `i2c-collision` | error / warning | Same address on one I2C bus (error when both confirmed). |
| `confirmed-needs-gpio` | error | Confirmed hardware without a pin. |
| `blocked-has-gpio` | warning | Blocked hardware that names a pin. |
| `evidence` | error / warning | Manifest GPIO differs from the firmware constant (error) or the symbol is missing (warning). |
| `markers` | warning | README/Wiring lacks the generated-block markers. |

## Markers

```markdown
<!-- GENIUS:PIN_MAP:START -->
<!-- GENIUS:PIN_MAP:END -->
```

in `README.md`, and

```markdown
<!-- GENIUS:CONNECTION_TABLE:START -->
<!-- GENIUS:CONNECTION_TABLE:END -->
```

in `docs/Wiring.md`. Everything between the markers is replaced; everything outside is left alone, so prose, wiring order, and electrical notes stay hand-written.

## What stays hand-authored

Flowcharts, control loops, and motor layouts describe algorithms rather than pins. They live in `docs/Implementation_Guide.md` and the other `images/*.md` files and are not touched by the generator.
