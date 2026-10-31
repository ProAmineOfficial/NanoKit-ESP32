#!/usr/bin/env python3
# Developed by Amine Saoud ibn al-Bashir.
# SPDX-License-Identifier: AGPL-3.0-only
"""Genius hardware generator for NanoKit-ESP32.

Reads `registry/nanokit-esp32.yaml` (the board as data) and every project
`nanokit.yaml` (the project hardware as data), validates them against the
ESP32 and NanoKit pin rules, cross-checks the GPIO constants in the firmware
source as evidence, and generates:

  <project>/images/connection-diagram.md   physical wiring, header pin + GPIO labels
  <project>/images/system-diagram.md       every component grouped by bus, with
                                           Confirmed / Proposed / Blocked styling
  <project>/README.md                      NanoKit Pin Mapping block (between markers)
  <project>/docs/Wiring.md                 Connection Table block (between markers)
  <project>/.genius/VALIDATION_REPORT.md   generator, command, inputs, time, result
  <project>/.genius/project.json           machine-readable manifest + findings
  <project>/.genius/README.md              status page for the .genius directory

Usage:
  python tools/genius/generate.py            # validate + generate every project
  python tools/genius/generate.py --check    # validate only, change nothing
  python tools/genius/generate.py --project examples_on_platformio/ultrasonic_distance
  python tools/genius/generate.py --render   # also render SVG with mmdc when available

Exit status is 1 when any project has a validation error, so CI fails.
Requires PyYAML: pip install pyyaml
"""

from __future__ import annotations

import argparse
import datetime as _dt
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

try:
    import yaml
except ImportError:  # pragma: no cover - friendly message for a fresh machine
    sys.stderr.write("PyYAML is required: pip install pyyaml\n")
    sys.exit(2)

GENERATOR_NAME = "NanoKit Genius Generator"
GENERATOR_VERSION = "1.0.0"
MANIFEST_NAME = "nanokit.yaml"
SCHEMA_VERSION = 1

ROOT = Path(__file__).resolve().parents[2]
REGISTRY_PATH = ROOT / "registry" / "nanokit-esp32.yaml"

PROJECT_AREAS = {
    "examples_on_platformio": "example",
    "applications_on_platformio": "application",
    "open_source_projects": "open_source",
    "graduation_projects": "graduation",
}

VALID_STATUS = {"confirmed", "proposed", "blocked"}
VALID_KINDS = {"sensor", "actuator", "driver", "display", "storage", "comm", "power", "node", "other"}
VALID_DIRS = {"out", "in", "bidir"}
VALID_SIGNAL_TYPES = {"gpio", "pwm", "adc", "dac", "uart", "spi", "i2s", "i2c", "touch"}

PIN_KEYS = {"pin", "gpio", "dir", "type", "symbol", "note", "level", "level_shift", "rail", "header"}
BUS_PIN_KEYS = {"role", "gpio", "symbol", "note", "header"}

MARK_README = ("<!-- GENIUS:PIN_MAP:START -->", "<!-- GENIUS:PIN_MAP:END -->")
MARK_WIRING = ("<!-- GENIUS:CONNECTION_TABLE:START -->", "<!-- GENIUS:CONNECTION_TABLE:END -->")

# Mermaid class definitions shared by every generated diagram (dark-theme friendly).
MERMAID_CLASSES = """  classDef controller fill:#102936,stroke:#4dd4ff,stroke-width:2px,color:#f4fbff
  classDef node fill:#12334c,stroke:#66d4ff,stroke-width:1.8px,color:#f3f7ff
  classDef sensor fill:#221b34,stroke:#ad8cff,stroke-width:1.8px,color:#f7f0ff
  classDef actuator fill:#37251a,stroke:#f0a560,stroke-width:1.8px,color:#fff8ef
  classDef driver fill:#2e2116,stroke:#f0a24a,stroke-width:1.8px,color:#fff6eb
  classDef storage fill:#2a2f3d,stroke:#9fb0c8,stroke-width:1.8px,color:#f3f7ff
  classDef display fill:#3c2349,stroke:#e3a0ed,stroke-width:1.8px,color:#f3f7ff
  classDef comm fill:#0d2929,stroke:#4ed6c4,stroke-width:1.8px,color:#edfffc
  classDef power fill:#183326,stroke:#7ccd91,stroke-width:1.8px,color:#f4fff6
  classDef bus fill:#1c2f4d,stroke:#8fb3ff,stroke-width:1.8px,color:#f3f7ff
  classDef external fill:#28263a,stroke:#9ea9ff,stroke-width:1.8px,color:#f7f6ff
  classDef other fill:#1f2430,stroke:#b0b8c6,stroke-width:1.6px,color:#f3f7ff
  classDef proposed stroke-dasharray:6 4,stroke:#e0c45b,color:#fff9de
  classDef blocked stroke-dasharray:3 3,stroke:#ef747d,color:#fff3f4
  linkStyle default stroke:#7894a5,stroke-width:1.4px"""


# --------------------------------------------------------------------------- data


@dataclass
class Finding:
    level: str  # error | warning | info
    rule: str
    message: str
    where: str = ""

    def as_dict(self) -> dict[str, str]:
        return {"level": self.level, "rule": self.rule, "message": self.message, "where": self.where}


@dataclass
class Evidence:
    symbol: str
    expected: int | None
    found: int | None
    location: str
    status: str  # match | mismatch | missing


@dataclass
class Project:
    path: Path
    manifest: dict[str, Any]
    findings: list[Finding] = field(default_factory=list)
    evidence: list[Evidence] = field(default_factory=list)
    inputs: list[Path] = field(default_factory=list)

    @property
    def rel(self) -> str:
        return self.path.relative_to(ROOT).as_posix()

    @property
    def name(self) -> str:
        return str(self.manifest.get("name") or self.path.name)

    def error(self, rule: str, message: str, where: str = "") -> None:
        self.findings.append(Finding("error", rule, message, where))

    def warn(self, rule: str, message: str, where: str = "") -> None:
        self.findings.append(Finding("warning", rule, message, where))

    def info(self, rule: str, message: str, where: str = "") -> None:
        self.findings.append(Finding("info", rule, message, where))

    @property
    def errors(self) -> list[Finding]:
        return [f for f in self.findings if f.level == "error"]

    @property
    def warnings(self) -> list[Finding]:
        return [f for f in self.findings if f.level == "warning"]


class Registry:
    """The NanoKit board as data."""

    def __init__(self, data: dict[str, Any]):
        self.data = data
        self.board = data["board"]
        self.rails: dict[str, dict[str, Any]] = data["rails"]
        self.pins: list[dict[str, Any]] = data["pins"]
        self.by_gpio: dict[int, dict[str, Any]] = {p["gpio"]: p for p in self.pins if p.get("gpio") is not None}
        self.by_header: dict[int, dict[str, Any]] = {p["header"]: p for p in self.pins}

    def pin(self, gpio: int) -> dict[str, Any] | None:
        return self.by_gpio.get(gpio)

    def header_of(self, gpio: int) -> int | None:
        pin = self.pin(gpio)
        return None if pin is None else int(pin["header"])

    def vr_of(self, gpio: int) -> str:
        pin = self.pin(gpio)
        return "-" if pin is None else str(pin.get("vr") or "-")

    def rail_headers(self, rail: str) -> str:
        info = self.rails.get(rail)
        if not info:
            return "-"
        return " or ".join(str(h) for h in info["header"])


# ------------------------------------------------------------------- discovery


def load_registry() -> Registry:
    with REGISTRY_PATH.open(encoding="utf-8") as handle:
        return Registry(yaml.safe_load(handle))


def discover_projects(explicit: list[str] | None) -> list[Project]:
    projects: list[Project] = []
    if explicit:
        candidates = [ROOT / item for item in explicit]
    else:
        candidates = []
        for area in PROJECT_AREAS:
            area_path = ROOT / area
            if area_path.exists():
                candidates.extend(sorted(p.parent for p in area_path.rglob(MANIFEST_NAME)))
    for folder in candidates:
        manifest_path = folder / MANIFEST_NAME
        if not manifest_path.exists():
            sys.stderr.write(f"skip {folder}: no {MANIFEST_NAME}\n")
            continue
        with manifest_path.open(encoding="utf-8") as handle:
            data = yaml.safe_load(handle) or {}
        project = Project(path=folder, manifest=data)
        project.inputs.append(manifest_path)
        projects.append(project)
    return projects


# ------------------------------------------------------------------ validation


def _components(project: Project) -> list[dict[str, Any]]:
    return list(project.manifest.get("components") or [])


def _buses(project: Project) -> list[dict[str, Any]]:
    return list(project.manifest.get("buses") or [])


def _firmwares(project: Project) -> list[dict[str, Any]]:
    firmwares = list(project.manifest.get("firmware") or [])
    if not firmwares:
        firmwares = [{"id": "main", "path": ".", "code_roots": ["src", "include"]}]
    return firmwares


def _firmware_of(project: Project, owner: dict[str, Any]) -> dict[str, Any]:
    firmwares = _firmwares(project)
    wanted = owner.get("firmware")
    for fw in firmwares:
        if fw.get("id") == wanted:
            return fw
    return firmwares[0]


def _status(item: dict[str, Any], default: str = "confirmed") -> str:
    return str(item.get("status") or default).lower()


def validate_schema(project: Project) -> None:
    m = project.manifest
    if m.get("genius") != SCHEMA_VERSION:
        project.error("schema", f"`genius: {SCHEMA_VERSION}` is required at the top of {MANIFEST_NAME}.")
    for key in ("name", "category", "board"):
        if not m.get(key):
            project.error("schema", f"`{key}` is required.")
    if m.get("category") and m["category"] not in PROJECT_AREAS.values():
        project.error("schema", f"category must be one of {sorted(PROJECT_AREAS.values())}, got {m['category']!r}.")
    area = project.path.relative_to(ROOT).parts[0]
    if PROJECT_AREAS.get(area) and m.get("category") and PROJECT_AREAS[area] != m["category"]:
        project.error("schema", f"category {m['category']!r} does not match the folder area {area!r}.")
    if m.get("board") != "nanokit-esp32":
        project.error("schema", "board must be `nanokit-esp32` (the registry id).")
    ids: set[str] = set()
    for comp in _components(project):
        cid = comp.get("id")
        if not cid:
            project.error("schema", "every component needs an `id`.")
            continue
        if cid in ids:
            project.error("schema", f"duplicate component id {cid!r}.")
        ids.add(cid)
        if comp.get("kind", "other") not in VALID_KINDS:
            project.error("schema", f"component {cid}: kind must be one of {sorted(VALID_KINDS)}.")
        if _status(comp) not in VALID_STATUS:
            project.error("schema", f"component {cid}: status must be one of {sorted(VALID_STATUS)}.")
        for pin in comp.get("pins") or []:
            unknown = set(pin) - PIN_KEYS
            if unknown:
                project.error("schema", f"component {cid} pin {pin.get('pin')}: unknown keys {sorted(unknown)} - a comma inside a flow-style note splits it; quote the note.")
            if "gpio" in pin and pin["gpio"] is not None and pin.get("dir", "in") not in VALID_DIRS:
                project.error("schema", f"component {cid} pin {pin.get('pin')}: dir must be one of {sorted(VALID_DIRS)}.")
            if pin.get("type", "gpio") not in VALID_SIGNAL_TYPES:
                project.error("schema", f"component {cid} pin {pin.get('pin')}: unknown type {pin.get('type')!r}.")
    bus_ids: set[str] = set()
    for bus in _buses(project):
        bid = bus.get("id")
        if not bid:
            project.error("schema", "every bus needs an `id`.")
            continue
        if bid in bus_ids:
            project.error("schema", f"duplicate bus id {bid!r}.")
        bus_ids.add(bid)
        if bus.get("type") not in {"i2c", "spi", "uart", "i2s", "onewire", "can"}:
            project.error("schema", f"bus {bid}: unsupported type {bus.get('type')!r}.")
    for comp in _components(project):
        if comp.get("bus") and comp["bus"] not in bus_ids:
            project.error("schema", f"component {comp.get('id')}: bus {comp['bus']!r} is not declared under `buses`.")
    for fw in _firmwares(project):
        fw_path = project.path / fw.get("path", ".")
        if not (fw_path / "platformio.ini").exists():
            project.warn("firmware", f"firmware {fw.get('id')}: no platformio.ini under {fw.get('path', '.')!r}.", fw.get("path", "."))


def _gpio_rules(project: Project, registry: Registry, gpio: int, direction: str, signal_type: str, wifi: bool, where: str) -> None:
    pin = registry.pin(gpio)
    if pin is None:
        project.error("pin-exists", f"GPIO{gpio} is not broken out on the NanoKit header.", where)
        return
    if pin.get("flash"):
        project.error("flash-pin", f"GPIO{gpio} (NanoKit Pin {pin['header']}) is the internal SPI flash. Never assign a signal to it.", where)
    if pin.get("input_only") and direction in {"out", "bidir"}:
        project.error("input-only", f"GPIO{gpio} (NanoKit Pin {pin['header']}) is input-only and cannot be driven as {direction}.", where)
    if pin.get("uart0"):
        project.error("uart0-shared", f"GPIO{gpio} (NanoKit Pin {pin['header']}) is UART0, shared with USB-serial flashing and Serial Monitor.", where)
    if gpio == 0:
        project.error("strapping", f"GPIO0 (NanoKit Pin 34) is the BOOT strapping pin. Do not use it for application signals.", where)
    elif pin.get("strapping_minor"):
        project.info("strapping", f"GPIO{gpio} (NanoKit Pin {pin['header']}): {pin['strapping_minor']}.", where)
    elif pin.get("strapping"):
        project.warn("strapping", f"GPIO{gpio} (NanoKit Pin {pin['header']}): {pin['strapping']}.", where)
    if signal_type == "adc":
        adc = pin.get("adc")
        if not adc:
            project.error("adc-capable", f"GPIO{gpio} has no ADC channel.", where)
        elif adc["unit"] == 2 and wifi:
            project.error("adc2-wifi", f"GPIO{gpio} is ADC2_CH{adc['ch']}; ADC2 is unreadable while Wi-Fi runs. Use an ADC1 pin (GPIO32-39).", where)
        elif adc["unit"] == 2:
            project.warn("adc2-wifi", f"GPIO{gpio} is ADC2; it stops working if Wi-Fi is enabled later.", where)
    if signal_type == "dac" and not pin.get("dac"):
        project.error("dac-capable", f"GPIO{gpio} has no DAC. Use GPIO25 (NanoKit Pin 3) or GPIO26 (NanoKit Pin 37).", where)
    if signal_type == "pwm" and pin.get("input_only"):
        project.error("input-only", f"GPIO{gpio} is input-only and cannot output PWM.", where)
    if signal_type == "touch" and pin.get("touch") is None:
        project.error("touch-capable", f"GPIO{gpio} has no touch channel.", where)


def validate_hardware(project: Project, registry: Registry) -> None:
    used: dict[tuple[str, int], str] = {}  # (firmware id, gpio) -> owner label

    def claim(fw_id: str, gpio: int, owner: str, where: str) -> None:
        key = (fw_id, gpio)
        if key in used:
            project.error("double-assigned", f"GPIO{gpio} is used by both {used[key]} and {owner} in firmware {fw_id!r}.", where)
        used[key] = owner

    for bus in _buses(project):
        fw = _firmware_of(project, bus)
        wifi = bool(fw.get("wifi"))
        bstatus = _status(bus)
        for bpin in bus.get("pins") or []:
            gpio = bpin.get("gpio")
            where = f"bus {bus['id']} {bpin.get('role')}"
            unknown = set(bpin) - BUS_PIN_KEYS
            if unknown:
                project.error("schema", f"{where}: unknown keys {sorted(unknown)} - a comma inside a flow-style note splits it; quote the note.", where)
            if gpio is None:
                if bstatus == "confirmed":
                    project.error("confirmed-needs-gpio", f"bus {bus['id']} is confirmed but {bpin.get('role')} has no gpio.", where)
                continue
            if bpin.get("header") is not None and registry.header_of(int(gpio)) != int(bpin["header"]):
                project.error("header-mismatch", f"{where}: NanoKit Pin {bpin['header']} does not carry GPIO{gpio} (registry says Pin {registry.header_of(int(gpio))}).", where)
            direction = "bidir" if bus["type"] in {"i2c", "onewire"} else ("out" if bpin.get("role", "").upper() in {"SCL", "SCLK", "SCK", "MOSI", "CS", "TX", "BCLK", "LRCLK", "DOUT"} else "in")
            _gpio_rules(project, registry, int(gpio), direction, bus["type"], wifi, where)
            claim(fw.get("id", "main"), int(gpio), f"bus {bus['id']} {bpin.get('role')}", where)

    for comp in _components(project):
        fw = _firmware_of(project, comp)
        wifi = bool(fw.get("wifi"))
        cstatus = _status(comp)
        cid = comp["id"] if comp.get("id") else "?"
        power = comp.get("power") or {}
        rail = power.get("rail")
        if rail and rail not in registry.rails and rail not in {"external", "none"}:
            project.error("rail", f"component {cid}: unknown power rail {rail!r}. Use one of {sorted(registry.rails)} or external.", cid)
        if comp.get("bus"):
            bus = next(b for b in _buses(project) if b["id"] == comp["bus"])
            if bus["type"] == "i2c" and not comp.get("address") and cstatus == "confirmed":
                project.warn("i2c-address", f"component {cid} is confirmed on I2C bus {bus['id']} but has no address.", cid)
        for pin in comp.get("pins") or []:
            where = f"{cid}.{pin.get('pin')}"
            if pin.get("rail"):
                if pin["rail"] not in registry.rails and pin["rail"] not in {"external"}:
                    project.error("rail", f"{where}: unknown rail {pin['rail']!r}.", where)
                continue
            gpio = pin.get("gpio")
            if gpio is None:
                if cstatus == "confirmed":
                    project.error("confirmed-needs-gpio", f"{where} is confirmed but has no gpio. Confirmed hardware must have a verified pin.", where)
                continue
            gpio = int(gpio)
            if cstatus == "blocked":
                project.warn("blocked-has-gpio", f"{where} is Blocked/TODO yet names GPIO{gpio}; treat it as a proposal only.", where)
            if pin.get("header") is not None and registry.header_of(gpio) != int(pin["header"]):
                project.error("header-mismatch", f"{where}: NanoKit Pin {pin['header']} does not carry GPIO{gpio} (registry says Pin {registry.header_of(gpio)}).", where)
            direction = str(pin.get("dir", "in"))
            signal_type = str(pin.get("type", "gpio"))
            _gpio_rules(project, registry, gpio, direction, signal_type, wifi, where)
            level = str(pin.get("level", "3V3")).upper().replace(".", "")
            if direction in {"in", "bidir"} and level in {"5V", "5V0"}:
                if pin.get("level_shift") not in {"required", "present", "divider", "shifter"}:
                    project.error("5v-input", f"{where}: a 5 V signal enters GPIO{gpio}; ESP32 is not 5 V tolerant. Declare `level_shift: required|present`.", where)
                else:
                    project.warn("5v-input", f"{where}: 5 V signal on GPIO{gpio} needs the declared level shifting ({pin['level_shift']}).", where)
            claim(fw.get("id", "main"), gpio, f"{cid}.{pin.get('pin')}", where)
            if registry.pin(gpio) and registry.pin(gpio).get("builtin_led") and comp.get("kind") != "display" and cid != "builtin_led":
                project.info("builtin-led", f"{where} uses GPIO2 (NanoKit Pin 17); the onboard LED will blink with this signal.", where)

    # I2C address collisions among confirmed devices.
    for bus in _buses(project):
        if bus.get("type") != "i2c":
            continue
        seen: dict[str, str] = {}
        for comp in _components(project):
            if comp.get("bus") != bus["id"] or not comp.get("address"):
                continue
            addr = str(comp["address"]).lower()
            if addr in seen:
                level = "error" if _status(comp) == "confirmed" and seen[addr].endswith("[confirmed]") else "warning"
                project.findings.append(Finding(level, "i2c-collision", f"I2C address {addr} on bus {bus['id']} is used by {seen[addr]} and {comp['id']} [{_status(comp)}]. Use a TCA9548A channel or re-address.", comp["id"]))
            seen[addr] = f"{comp['id']} [{_status(comp)}]"


# -------------------------------------------------------------------- evidence

_DEFINE_RE = re.compile(r"^\s*#define\s+(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s+\(?\s*(?P<value>\d+)\s*\)?\s*(?://.*)?$")
_CONST_RE = re.compile(r"^\s*(?:static\s+)?(?:constexpr|const)\s+(?:uint8_t|uint16_t|int|gpio_num_t|unsigned|uint32_t|size_t)\s+(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*=\s*\(?\s*(?:GPIO_NUM_)?(?P<value>\d+)\s*\)?\s*[;,]")


_ARRAY_RE = re.compile(
    r"(?:constexpr|const)\s+(?:std::array<\s*\w+\s*,\s*\d+\s*>|(?:uint8_t|int|gpio_num_t))\s+(?P<name>[A-Za-z_][A-Za-z0-9_]*)(?:\[\s*\d*\s*\])?\s*=\s*\{(?P<body>[^}]*)\}",
    re.S,
)


def _scan_symbols(folder: Path) -> dict[str, tuple[int, str]]:
    """Map NAME -> (value, file:line) for scalar constants and NAME[i] for array initialisers."""
    table: dict[str, tuple[int, str]] = {}
    if not folder.exists():
        return table
    for file in sorted(folder.rglob("*")):
        if file.suffix.lower() not in {".h", ".hpp", ".cpp", ".c", ".ino"}:
            continue
        try:
            text = file.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        rel = file.relative_to(ROOT).as_posix()
        for number, line in enumerate(text.splitlines(), start=1):
            match = _DEFINE_RE.match(line) or _CONST_RE.match(line)
            if match:
                table.setdefault(match.group("name"), (int(match.group("value")), f"{rel}:{number}"))
        for match in _ARRAY_RE.finditer(text):
            line_number = text.count("\n", 0, match.start()) + 1
            body = re.sub(r"//[^\n]*", "", match.group("body"))
            values = [item.strip() for item in body.split(",") if item.strip()]
            for index, value in enumerate(values):
                if value.isdigit():
                    table.setdefault(f"{match.group('name')}[{index}]", (int(value), f"{rel}:{line_number}"))
    return table


def check_evidence(project: Project) -> None:
    symbol_tables: dict[str, dict[str, tuple[int, str]]] = {}
    for fw in _firmwares(project):
        table: dict[str, tuple[int, str]] = {}
        for root in fw.get("code_roots") or ["src", "include"]:
            folder = project.path / fw.get("path", ".") / root
            table.update(_scan_symbols(folder))
            if folder.exists():
                project.inputs.extend(sorted(p for p in folder.rglob("*") if p.suffix.lower() in {".h", ".hpp", ".cpp", ".c", ".ino"}))
        symbol_tables[fw.get("id", "main")] = table

    def check(owner: dict[str, Any], pin: dict[str, Any], where: str) -> None:
        symbol = pin.get("symbol")
        gpio = pin.get("gpio")
        if gpio is None:
            return
        fw_id = _firmware_of(project, owner).get("id", "main")
        table = symbol_tables.get(fw_id, {})
        if not symbol:
            if _status(owner) == "confirmed":
                project.warn("evidence", f"{where}: confirmed GPIO{gpio} has no `symbol`, so the code cannot be cross-checked.", where)
            return
        if symbol not in table:
            project.warn("evidence", f"{where}: symbol {symbol} not found in firmware {fw_id!r} sources.", where)
            project.evidence.append(Evidence(symbol, int(gpio), None, "-", "missing"))
            return
        value, location = table[symbol]
        if value != int(gpio):
            project.error("evidence", f"{where}: manifest says GPIO{gpio} but {symbol} = {value} at {location}.", where)
            project.evidence.append(Evidence(symbol, int(gpio), value, location, "mismatch"))
        else:
            project.evidence.append(Evidence(symbol, int(gpio), value, location, "match"))

    for bus in _buses(project):
        for bpin in bus.get("pins") or []:
            check(bus, bpin, f"bus {bus['id']} {bpin.get('role')}")
    for comp in _components(project):
        for pin in comp.get("pins") or []:
            if pin.get("rail"):
                continue
            check(comp, pin, f"{comp.get('id')}.{pin.get('pin')}")


# ------------------------------------------------------------------- rendering


def _q(text: Any) -> str:
    """Make text safe inside a quoted Mermaid label."""
    return str(text).replace('"', "'").replace("#", "No.").replace("\n", " ").strip()


def _node_id(prefix: str, raw: str) -> str:
    return prefix + re.sub(r"[^A-Za-z0-9_]", "_", raw)


def _status_class(status: str) -> str:
    """Return a second `class` statement suffix for non-confirmed items (Mermaid accumulates classes)."""
    return {"proposed": "\n  class {node} proposed", "blocked": "\n  class {node} blocked"}.get(status, "")


def _class_lines(node: str, base: str, status: str) -> str:
    return f"  class {node} {base}" + _status_class(status).format(node=node)


def _status_tag(status: str) -> str:
    return {"confirmed": "", "proposed": " · proposed", "blocked": " · blocked/TODO"}[status]


def _pin_label(registry: Registry, gpio: int) -> str:
    header = registry.header_of(gpio)
    return f"Pin {header} / GPIO{gpio}" if header is not None else f"GPIO{gpio}"


def _edge(status: str) -> tuple[str, str, str]:
    """Return (arrow-out, arrow-in, line) styles for a status."""
    if status == "confirmed":
        return "-->", "-->", "---"
    return "-.->", "-.->", "-.-"


def render_connection_diagram(project: Project, registry: Registry) -> str:
    lines = ["flowchart LR"]
    firmwares = _firmwares(project)
    for fw in firmwares:
        nid = _node_id("FW_", fw.get("id", "main"))
        label = fw.get("name") or ("NanoKit Integrated ESP32" if len(firmwares) == 1 else f"NanoKit ESP32 · {fw.get('id')}")
        lines.append(f'  {nid}(["{_q(label)}"])')
    rails_used: set[str] = set()
    class_lines: list[str] = []
    for bus in _buses(project):
        bid = _node_id("BUS_", bus["id"])
        fw = _node_id("FW_", _firmware_of(project, bus).get("id", "main"))
        bstatus = _status(bus)
        pins_text = ", ".join(f"{bp.get('role')} {_pin_label(registry, int(bp['gpio']))}" for bp in bus.get("pins") or [] if bp.get("gpio") is not None)
        extra = f" · {bus['speed']}" if bus.get("speed") else ""
        lines.append(f'  {bid}[["{_q(bus["type"].upper() + " bus " + bus["id"] + extra + _status_tag(bstatus))}"]]')
        out_arrow, _, line_style = _edge(bstatus)
        lines.append(f'  {fw} {line_style}|"{_q(pins_text) or "pins TODO"}"| {bid}')
        class_lines.append(_class_lines(bid, f"bus", bstatus))
    linked_ids = {link.get("from") for link in project.manifest.get("links") or []} | {link.get("to") for link in project.manifest.get("links") or []}
    for comp in _components(project):
        cid = _node_id("C_", comp["id"])
        cstatus = _status(comp)
        kind = comp.get("kind", "other")
        fw = _node_id("FW_", _firmware_of(project, comp).get("id", "main"))
        if not comp.get("bus") and not (comp.get("pins") or []) and comp["id"] not in linked_ids:
            # Ownership edge for a component that has no wiring yet, so it is never orphaned in the picture.
            lines.append(f'  {fw} -.-|"pins TODO"| {cid}' if cstatus != "confirmed" else f"  {fw} --- {cid}")
        label = f"{comp.get('name', comp['id'])}{_status_tag(cstatus)}"
        if comp.get("address"):
            label += f" @ {comp['address']}"
        shape = {"sensor": '[/"{}"/]', "actuator": '[/"{}"\\]', "driver": '["{}"]', "display": '[["{}"]]', "storage": '[("{}")]', "comm": '(["{}"])', "power": '{{"{}"}}', "node": '(["{}"])'}.get(kind, '["{}"]')
        lines.append(f"  {cid}{shape.format(_q(label))}")
        class_lines.append(_class_lines(cid, f"{kind if kind in {'sensor','actuator','driver','display','storage','comm','power','node'} else 'other'}", cstatus))
        out_arrow, in_arrow, line_style = _edge(cstatus)
        if comp.get("bus"):
            bid = _node_id("BUS_", comp["bus"])
            lines.append(f'  {bid} {line_style} {cid}')
        for pin in comp.get("pins") or []:
            if pin.get("rail"):
                rails_used.add(pin["rail"])
                rid = _node_id("RAIL_", pin["rail"])
                lines.append(f'  {rid} {line_style}|"{_q(pin.get("pin"))}"| {cid}')
                continue
            gpio = pin.get("gpio")
            if gpio is None:
                lines.append(f'  {fw} -.-|"{_q(pin.get("pin"))} · pin TODO"| {cid}')
                continue
            label = f"{_pin_label(registry, int(gpio))} · {pin.get('pin')}"
            if pin.get("level_shift"):
                label += " · level shift"
            direction = pin.get("dir", "in")
            if direction == "out":
                lines.append(f'  {fw} {out_arrow}|"{_q(label)}"| {cid}')
            elif direction == "in":
                lines.append(f'  {cid} {in_arrow}|"{_q(label)}"| {fw}')
            else:
                lines.append(f'  {fw} {line_style}|"{_q(label)}"| {cid}')
    for rail in sorted(rails_used):
        rid = _node_id("RAIL_", rail)
        info = registry.rails.get(rail)
        text = f"{rail} · Pin {registry.rail_headers(rail)}" if info else f"{rail} (external supply, share GND)"
        lines.append(f'  {rid}{{{{"{_q(text)}"}}}}')
        class_lines.append(f"  class {rid} power")
    for ext in project.manifest.get("externals") or []:
        eid = _node_id("X_", ext["id"])
        lines.append(f'  {eid}(["{_q(ext.get("name", ext["id"]))}"])')
        class_lines.append(f"  class {eid} external")
    lines.extend(_render_links(project))
    lines.append("")
    lines.append(MERMAID_CLASSES)
    for fw in firmwares:
        lines.append(f"  class {_node_id('FW_', fw.get('id', 'main'))} controller")
    lines.extend(class_lines)
    return "\n".join(lines)


def _render_links(project: Project) -> list[str]:
    firmwares = _firmwares(project)
    externals = project.manifest.get("externals") or []
    out: list[str] = []

    def resolve(identifier: str) -> str:
        if any(fw.get("id") == identifier for fw in firmwares):
            return _node_id("FW_", identifier)
        if any(ext.get("id") == identifier for ext in externals):
            return _node_id("X_", identifier)
        return _node_id("C_", identifier)

    for link in project.manifest.get("links") or []:
        arrow = {"both": "<-->", "one": "-->", "dashed": "-.->", "line": "---"}.get(link.get("kind", "one"), "-->")
        label = f'|"{_q(link["label"])}"|' if link.get("label") else ""
        out.append(f"  {resolve(link['from'])} {arrow}{label} {resolve(link['to'])}")
    return out


SUBGRAPH_STYLE = "fill:#0b1118,stroke:#2f3f4f,stroke-width:1px,color:#cfe3f0"


def render_system_diagram(project: Project, registry: Registry) -> str:
    """One block per NanoKit node: its buses (with attached devices) and its direct peripherals grouped by kind.

    Every component in the manifest is drawn, including Proposed and Blocked ones, so planned hardware is
    never silently missing from the picture (the drift that hand-drawn diagrams suffer from).
    """
    lines = ["flowchart TB"]
    firmwares = _firmwares(project)
    class_lines: list[str] = []
    style_lines: list[str] = []
    titles = {"sensor": "Sensors", "actuator": "Actuators", "driver": "Drivers", "display": "Indicators / displays", "storage": "Storage", "comm": "Communication", "power": "Power", "node": "Nodes", "other": "Other"}

    for ext in project.manifest.get("externals") or []:
        eid = _node_id("X_", ext["id"])
        lines.append(f'  {eid}(["{_q(ext.get("name", ext["id"]))}"])')
        class_lines.append(f"  class {eid} external")

    for fw in firmwares:
        fw_id = fw.get("id", "main")
        fw_node = _node_id("FW_", fw_id)
        group = _node_id("FWG_", fw_id)
        title = fw.get("name") or "NanoKit Integrated ESP32"
        if fw.get("role"):
            title += f" · {fw['role']}"
        lines.append(f'  subgraph {group}["{_q(title)}"]')
        lines.append("    direction TB")
        lines.append(f'    {fw_node}(["{_q("NanoKit Integrated ESP32" if len(firmwares) == 1 else fw.get("name", fw_id))}"])')
        class_lines.append(f"  class {fw_node} controller")
        style_lines.append(f"  style {group} {SUBGRAPH_STYLE}")

        for bus in _buses(project):
            if _firmware_of(project, bus).get("id", "main") != fw_id:
                continue
            bid = _node_id("BUS_", bus["id"])
            bstatus = _status(bus)
            pins_text = ", ".join(f"{bp.get('role')} {_pin_label(registry, int(bp['gpio']))}" for bp in bus.get("pins") or [] if bp.get("gpio") is not None) or "pins TODO"
            sg = f"SG_{bid}"
            lines.append(f'    subgraph {sg}["{_q(bus["type"].upper() + " " + bus["id"] + " · " + pins_text + _status_tag(bstatus))}"]')
            lines.append(f'      {bid}[["{_q(bus["type"].upper() + " bus")}"]]')
            class_lines.append(_class_lines(bid, "bus", bstatus))
            style_lines.append(f"  style {sg} {SUBGRAPH_STYLE}")
            for comp in _components(project):
                if comp.get("bus") != bus["id"]:
                    continue
                cid = _node_id("C_", comp["id"])
                cstatus = _status(comp)
                label = comp.get("name", comp["id"]) + (f"<br/>@ {comp['address']}" if comp.get("address") else "") + _status_tag(cstatus)
                lines.append(f'      {cid}["{_q(label)}"]')
                class_lines.append(_class_lines(cid, comp.get("kind", "other"), cstatus))
                lines.append(f"      {bid} {_edge(cstatus)[2]} {cid}")
            lines.append("    end")
            lines.append(f"    {fw_node} {_edge(bstatus)[2]} {bid}")

        grouped: dict[str, list[dict[str, Any]]] = {}
        for comp in _components(project):
            if comp.get("bus") or _firmware_of(project, comp).get("id", "main") != fw_id:
                continue
            grouped.setdefault(comp.get("kind", "other"), []).append(comp)
        for kind, comps in grouped.items():
            sg = f"SG_{fw_node}_{kind}"
            lines.append(f'    subgraph {sg}["{titles.get(kind, kind.title())}"]')
            style_lines.append(f"  style {sg} {SUBGRAPH_STYLE}")
            for comp in comps:
                cid = _node_id("C_", comp["id"])
                cstatus = _status(comp)
                pins = [p for p in comp.get("pins") or [] if not p.get("rail")]
                pin_text = ", ".join(f"{p.get('pin')} {_pin_label(registry, int(p['gpio']))}" if p.get("gpio") is not None else f"{p.get('pin')} TODO" for p in pins)
                label = comp.get("name", comp["id"]) + _status_tag(cstatus) + (f"<br/>{pin_text}" if pin_text else "")
                lines.append(f'      {cid}["{_q(label)}"]')
                class_lines.append(_class_lines(cid, kind, cstatus))
            lines.append("    end")
            for comp in comps:
                cid = _node_id("C_", comp["id"])
                cstatus = _status(comp)
                if comp.get("kind") == "power":
                    continue  # power chain is drawn by explicit links
                dirs = {p.get("dir", "in") for p in comp.get("pins") or [] if p.get("gpio") is not None}
                out_arrow, in_arrow, line_style = _edge(cstatus)
                if dirs == {"out"}:
                    lines.append(f"    {fw_node} {out_arrow} {cid}")
                elif dirs == {"in"}:
                    lines.append(f"    {cid} {in_arrow} {fw_node}")
                else:
                    lines.append(f"    {fw_node} {line_style} {cid}")
        lines.append("  end")

    lines.extend(_render_links(project))
    lines.append("")
    lines.append(MERMAID_CLASSES)
    lines.extend(class_lines)
    lines.extend(style_lines)
    return "\n".join(lines)


def render_pin_rows(project: Project, registry: Registry, style: str) -> list[str]:
    """style = 'readme' | 'wiring'. Returns markdown table lines."""
    rows: list[tuple[int, list[str]]] = []
    multi = len(_firmwares(project)) > 1

    def owner_prefix(owner: dict[str, Any]) -> str:
        return f"[{_firmware_of(project, owner).get('id')}] " if multi else ""

    def electrical(pin: dict[str, Any], gpio: int) -> str:
        notes: list[str] = []
        reg = registry.pin(gpio) or {}
        if reg.get("input_only"):
            notes.append("input-only pin")
        if pin.get("level_shift"):
            notes.append(f"{pin.get('level', '5V')} signal: level shift {pin['level_shift']}")
        else:
            notes.append("3.3 V logic")
        if reg.get("strapping") and not reg.get("strapping_minor"):
            notes.append("strapping pin, see registry")
        if pin.get("type") == "adc" and reg.get("adc"):
            notes.append(f"ADC{reg['adc']['unit']}_CH{reg['adc']['ch']}")
        if pin.get("type") == "pwm":
            notes.append("PWM / LEDC")
        return "; ".join(notes)

    for bus in _buses(project):
        for bpin in bus.get("pins") or []:
            gpio = bpin.get("gpio")
            if gpio is None:
                continue
            gpio = int(gpio)
            header = registry.header_of(gpio)
            role = str(bpin.get("role"))
            direction = "Bidirectional" if bus["type"] in {"i2c", "onewire"} else ("Output" if role.upper() in {"SCL", "SCLK", "SCK", "MOSI", "CS", "TX"} else "Input")
            devices = ", ".join(c.get("name", c["id"]) for c in _components(project) if c.get("bus") == bus["id"]) or "bus"
            status = _status(bus)
            if style == "readme":
                rows.append((header or 999, [str(header or "-"), f"GPIO{gpio}", f"{owner_prefix(bus)}{bus['type'].upper()} {role}{_status_tag(status)}", direction, bpin.get("note") or f"{bus['type'].upper()} bus {bus['id']} shared by {devices}", "3.3 V logic; external pull-ups per module" if bus["type"] == "i2c" else "3.3 V logic"]))
            else:
                rows.append((header or 999, [str(header or "-"), f"GPIO{gpio}", registry.vr_of(gpio), f"{owner_prefix(bus)}{bus['type'].upper()} bus {bus['id']}", role, direction, status.title(), bpin.get("note") or f"Shared by {devices}"]))
    many = len(_components(project)) > 1
    rail_order = {"GND": 900, "3V3": 901, "5V": 902}
    rail_notes = {"GND": "Common ground required", "3V3": "3.3 V rail, approx. 0.5-1 W available", "5V": "5 V rail; never feed 5 V signals into a GPIO without level shifting"}
    for comp in _components(project):
        cstatus = _status(comp)
        for pin in comp.get("pins") or []:
            if pin.get("rail"):
                rail = pin["rail"]
                header_text = registry.rail_headers(rail) if rail in registry.rails else "-"
                rail_cell = rail if rail in registry.rails else "External"
                comp_pin = f"{comp.get('name', comp['id'])} {pin.get('pin')}" if many else str(pin.get("pin"))
                if style == "readme":
                    rows.append((rail_order.get(rail, 903), [header_text, rail_cell, f"{owner_prefix(comp)}{comp_pin}", "Power", pin.get("note") or registry.rails.get(rail, {}).get("note", "External supply"), rail_notes.get(rail, "External supply, share GND with NanoKit")]))
                else:
                    rows.append((rail_order.get(rail, 903), [header_text, rail_cell, registry.rails.get(rail, {}).get("vr", "-") or "-", f"{owner_prefix(comp)}{comp.get('name', comp['id'])}", str(pin.get("pin")), "Power", cstatus.title(), pin.get("note") or rail_notes.get(rail, "External supply")]))
                continue
            gpio = pin.get("gpio")
            if gpio is None:
                if style == "wiring":
                    rows.append((998, ["TODO", "TODO", "-", comp.get("name", comp["id"]), str(pin.get("pin")), {"out": "Output", "in": "Input"}.get(pin.get("dir", "in"), "Bidirectional"), cstatus.title(), pin.get("note") or "Pin not assigned yet"]))
                continue
            gpio = int(gpio)
            header = registry.header_of(gpio)
            direction = {"out": "Output", "in": "Input"}.get(pin.get("dir", "in"), "Bidirectional")
            if style == "readme":
                rows.append((header or 999, [str(header or "-"), f"GPIO{gpio}", f"{owner_prefix(comp)}{pin.get('pin')}{_status_tag(cstatus)}", direction, pin.get("note") or comp.get("name", comp["id"]), electrical(pin, gpio)]))
            else:
                rows.append((header or 999, [str(header or "-"), f"GPIO{gpio}", registry.vr_of(gpio), f"{owner_prefix(comp)}{comp.get('name', comp['id'])}", str(pin.get("pin")), direction, cstatus.title(), pin.get("note") or electrical(pin, gpio)]))
    rows.sort(key=lambda item: 0 if item[0] < 900 else item[0])  # declaration order for signals, rails last
    if style == "readme":
        header = ["| NanoKit Pin | GPIO | Component Pin | Direction | Reason | Electrical Notes |", "|---:|---|---|---|---|---|"]
    else:
        header = ["| NanoKit Pin | GPIO | VR Address | Component | Component Pin | Direction | Status | Description |", "|---:|---|---|---|---|---|---|---|"]
    body = ["| " + " | ".join(cell.replace("|", "/") for cell in row) + " |" for _, row in rows]
    if not body:
        body = ["| - | - | no board pins declared in nanokit.yaml | - | - | - |" if style == "readme" else "| - | - | - | no board pins declared in nanokit.yaml | - | - | - | - |"]
    return header + body


# ---------------------------------------------------------------------- output


def _stamp() -> str:
    return _dt.datetime.now(_dt.timezone.utc).strftime("%Y-%m-%d %H:%M:%S UTC")


_VOLATILE = [
    re.compile(r"\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2} UTC"),     # generation time
    re.compile(r"(Command: `|\"command\": \")[^`\"]*"),           # how the run was invoked
]


def _normalise(text: str) -> str:
    for pattern in _VOLATILE:
        text = pattern.sub("<volatile>", text)
    return text


def write_if_changed(target: Path, content: str) -> bool:
    """Write `content` only when it differs from the file beyond its timestamp and command line.

    This keeps generated files byte-stable between runs, so the pre-commit hook and the
    GitHub workflow do not create a new commit just because the clock moved.
    """
    if target.exists():
        try:
            if _normalise(target.read_text(encoding="utf-8")) == _normalise(content):
                return False
        except OSError:
            pass
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(content, encoding="utf-8", newline="\n")
    return True


def _header_comment(project: Project) -> str:
    return (
        f"<!-- Generated by {GENERATOR_NAME} {GENERATOR_VERSION} from {project.rel}/{MANIFEST_NAME} "
        f"and registry/nanokit-esp32.yaml. Edit the manifest, not this file. -->"
    )


def write_diagram(project: Project, name: str, title: str, body: str, footer: str, dry: bool) -> Path:
    images_dir = project.path / (project.manifest.get("docs", {}) or {}).get("images", "images")
    target = images_dir / f"{name}.md"
    content = (
        f"# {title} - {project.name}\n\n"
        f"**Developed by Amine Saoud ibn al-Bashir.**\n\n"
        f"{_header_comment(project)}\n\n"
        f"```mermaid\n{body}\n```\n\n{footer}\n"
    )
    if not dry:
        write_if_changed(target, content)
    return target


def replace_block(project: Project, relpath: str, markers: tuple[str, str], block: str, dry: bool) -> None:
    target = project.path / relpath
    if not target.exists():
        project.warn("markers", f"{relpath} does not exist; block not written.", relpath)
        return
    text = target.read_text(encoding="utf-8")
    start, end = markers
    i = text.find(start)
    j = text.find(end)
    if i < 0 or j < 0 or j < i:
        project.warn("markers", f"{relpath} has no {start} / {end} markers; add them around the table to let Genius keep it current.", relpath)
        return
    updated = text[:i] + start + "\n" + block.rstrip("\n") + "\n" + text[j:]
    if updated != text and not dry:
        target.write_text(updated, encoding="utf-8", newline="\n")


def classify(project: Project) -> dict[str, list[str]]:
    buckets: dict[str, list[str]] = {"confirmed": [], "proposed": [], "blocked": []}
    for bus in _buses(project):
        buckets[_status(bus)].append(f"bus {bus['id']} ({bus['type'].upper()})")
    for comp in _components(project):
        buckets[_status(comp)].append(comp.get("name", comp["id"]))
    return buckets


def write_genius_reports(project: Project, registry: Registry, command: str, dry: bool) -> None:
    genius_dir = project.path / ".genius"
    buckets = classify(project)
    inputs = []
    for path in sorted(set(project.inputs)):
        try:
            # Hash the content as git stores it (LF), so Windows (CRLF) and Linux runs agree.
            digest = hashlib.sha256(path.read_bytes().replace(b"\r\n", b"\n")).hexdigest()[:12]
        except OSError:
            digest = "unreadable"
        inputs.append(f"- `{path.relative_to(ROOT).as_posix()}` · sha256 {digest}")
    status_word = "FAILED" if project.errors else ("PASSED WITH WARNINGS" if project.warnings else "PASSED")
    lines = [
        f"# Genius Validation Report - {project.name}",
        "",
        "**Developed by Amine Saoud ibn al-Bashir.**",
        "",
        f"- Generator: {GENERATOR_NAME} {GENERATOR_VERSION}",
        f"- Command: `{command}`",
        f"- Generated: {_stamp()}",
        f"- Board registry: `registry/nanokit-esp32.yaml` ({registry.board['name']} {registry.board['revision']})",
        f"- Result: **{status_word}** ({len(project.errors)} errors, {len(project.warnings)} warnings)",
        "",
        "## Inputs",
        "",
        *inputs,
        "",
        "## Hardware Classification",
        "",
        f"- Confirmed: {', '.join(buckets['confirmed']) or 'none'}",
        f"- Proposed: {', '.join(buckets['proposed']) or 'none'}",
        f"- Blocked/TODO: {', '.join(buckets['blocked']) or 'none'}",
        "",
        "## Rules Checked",
        "",
        "flash-pin (GPIO6-11), input-only (GPIO34/35/36/39), uart0-shared (GPIO1/3), strapping (GPIO0/2/12/15), "
        "adc2-wifi, dac-capable, 5v-input level shifting, double-assigned GPIO, header-mismatch, i2c-collision, "
        "confirmed-needs-gpio, evidence (manifest GPIO vs firmware constant).",
        "",
        "## Findings",
        "",
    ]
    if project.findings:
        lines.append("| Level | Rule | Where | Message |")
        lines.append("|---|---|---|---|")
        for finding in project.findings:
            lines.append(f"| {finding.level} | {finding.rule} | {finding.where or '-'} | {finding.message.replace('|', '/')} |")
    else:
        lines.append("No findings.")
    lines += ["", "## Code Evidence", ""]
    if project.evidence:
        lines.append("| Symbol | Manifest GPIO | Code value | Location | Status |")
        lines.append("|---|---:|---:|---|---|")
        for item in project.evidence:
            lines.append(f"| `{item.symbol}` | {item.expected if item.expected is not None else '-'} | {item.found if item.found is not None else '-'} | {item.location} | {item.status} |")
    else:
        lines.append("No pin symbols declared; add `symbol:` to each pin in nanokit.yaml to enable cross-checking.")
    lines += [
        "",
        "## Known Limitations",
        "",
        "- The generator proves consistency between manifest, registry, and source constants. It does not prove that the hardware was bench-tested; the `status:` field is a human statement of evidence.",
        "- Flowcharts describe algorithms and remain hand-authored in `docs/Implementation_Guide.md`.",
        "",
    ]
    report = "\n".join(lines)
    payload = {
        "generator": {"name": GENERATOR_NAME, "version": GENERATOR_VERSION},
        "generated_at": _stamp(),
        "project": project.rel,
        "manifest": project.manifest,
        "board": {"id": registry.board["id"], "name": registry.board["name"], "revision": registry.board["revision"]},
        "classification": buckets,
        "findings": [f.as_dict() for f in project.findings],
        "evidence": [item.__dict__ for item in project.evidence],
        "result": status_word,
    }
    readme = "\n".join([
        "# Genius Guides",
        "",
        "**Developed by Amine Saoud ibn al-Bashir.**",
        "",
        "## Current Status",
        "",
        f"This directory is written by {GENERATOR_NAME} {GENERATOR_VERSION} (`tools/genius/generate.py`). "
        f"Last run: {_stamp()} — result **{status_word}**.",
        "",
        "Inputs: `nanokit.yaml` in this project, `registry/nanokit-esp32.yaml`, and the firmware source constants listed in the report.",
        "",
        "## Files",
        "",
        "- `VALIDATION_REPORT.md` — generator, command, inputs, rules, findings, code evidence.",
        "- `project.json` — the same data for machines (Git Architecture Diagram, Genius, CI).",
        "",
        "## Generated Elsewhere In This Project",
        "",
        "- `images/connection-diagram.md` and `images/system-diagram.md` are fully generated.",
        "- The pin tables between `GENIUS:*` markers in `README.md` and `docs/Wiring.md` are generated.",
        "",
        "Edit `nanokit.yaml`, then run `python tools/genius/generate.py`. Do not edit generated files by hand.",
        "",
    ])
    if not dry:
        payload["command"] = command
        write_if_changed(genius_dir / "VALIDATION_REPORT.md", report)
        write_if_changed(genius_dir / "project.json", json.dumps(payload, indent=2, ensure_ascii=False) + "\n")
        write_if_changed(genius_dir / "README.md", readme)


def render_svg(markdown_file: Path) -> bool:
    mmdc = shutil.which("mmdc")
    if not mmdc:
        return False
    text = markdown_file.read_text(encoding="utf-8")
    match = re.search(r"```mermaid\s*\n(.*?)```", text, re.S)
    if not match:
        return False
    tmp = markdown_file.with_suffix(".mmd")
    tmp.write_text(match.group(1), encoding="utf-8")
    try:
        subprocess.run([mmdc, "-i", str(tmp), "-o", str(markdown_file.with_suffix(".svg")), "-b", "transparent", "-q"], check=True, timeout=120)
        return True
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired):
        return False
    finally:
        tmp.unlink(missing_ok=True)


# ------------------------------------------------------------------------ main


def process(project: Project, registry: Registry, command: str, check_only: bool, render: bool) -> None:
    validate_schema(project)
    if not project.errors:
        validate_hardware(project, registry)
        check_evidence(project)
    if check_only:
        return
    docs = project.manifest.get("docs") or {}
    connection = render_connection_diagram(project, registry)
    system = render_system_diagram(project, registry)
    buckets = classify(project)
    legend = ("Solid lines and plain labels are **Confirmed**; dashed yellow nodes are **Proposed**; dashed red nodes are **Blocked/TODO**. "
              "Labels read `Pin <NanoKit pin> / GPIO<n>` so the physical NanoKit header pin and the ESP32 GPIO are always shown together.")
    c_path = write_diagram(project, "connection-diagram", "Connection Diagram", connection, legend + " Render to PNG/SVG with `python tools/genius/generate.py --render`.", False)
    s_path = write_diagram(project, "system-diagram", "System Diagram", system,
                           legend + f" Every component declared in `nanokit.yaml` appears here, including those still blocked ({len(buckets['blocked'])}) or proposed ({len(buckets['proposed'])}), so the diagram never silently drops planned hardware.", False)
    readme_block = "\n".join(render_pin_rows(project, registry, "readme"))
    wiring_block = "\n".join(render_pin_rows(project, registry, "wiring"))
    replace_block(project, docs.get("readme", "README.md"), MARK_README, readme_block, False)
    replace_block(project, docs.get("wiring", "docs/Wiring.md"), MARK_WIRING, wiring_block, False)
    write_genius_reports(project, registry, command, False)
    if render:
        for path in (c_path, s_path):
            if not render_svg(path):
                project.info("render", f"SVG not rendered for {path.name} (mmdc missing or failed).")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--project", action="append", help="project folder relative to the repository root (repeatable)")
    parser.add_argument("--check", action="store_true", help="validate only; write nothing")
    parser.add_argument("--render", action="store_true", help="also render SVG files with mmdc when it is installed")
    parser.add_argument("--json", action="store_true", help="print a JSON summary to stdout")
    args = parser.parse_args(argv)
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8", errors="replace")
        except (AttributeError, ValueError):
            pass
    command = "python tools/genius/generate.py" + ("".join(f" --project {p}" for p in args.project or [])) + (" --check" if args.check else "") + (" --render" if args.render else "")
    os.chdir(ROOT)
    registry = load_registry()
    projects = discover_projects(args.project)
    if not projects:
        print(f"No {MANIFEST_NAME} found under {', '.join(PROJECT_AREAS)}.")
        return 0
    total_errors = 0
    summary = []
    for project in projects:
        process(project, registry, command, args.check, args.render)
        total_errors += len(project.errors)
        verdict = "FAIL" if project.errors else ("WARN" if project.warnings else "OK")
        print(f"[{verdict}] {project.rel}: {len(project.errors)} errors, {len(project.warnings)} warnings, {len(project.evidence)} evidence checks")
        for finding in project.findings:
            if finding.level != "info":
                print(f"    {finding.level:7} {finding.rule:18} {finding.where:28} {finding.message}")
        summary.append({"project": project.rel, "result": verdict, "errors": len(project.errors), "warnings": len(project.warnings)})
    if args.json:
        print(json.dumps(summary, indent=2))
    return 1 if total_errors else 0


if __name__ == "__main__":
    sys.exit(main())
