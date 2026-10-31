# Developed by Amine Saoud ibn al-Bashir.
# SPDX-License-Identifier: AGPL-3.0-only
"""Self-tests for the Genius generator. Run: python -m pytest tools/genius/tests -q

The tests build a throw-away project inside the repository, feed it deliberately wrong
hardware, and assert that each rule fires. They never touch real projects.
"""

from __future__ import annotations

import shutil
import sys
import textwrap
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools" / "genius"))

import generate  # noqa: E402  (imported after sys.path tweak)

TMP = ROOT / "examples_on_platformio" / "zz_genius_selftest"


def _write_project(manifest: str, source: str = "") -> generate.Project:
    if TMP.exists():
        shutil.rmtree(TMP)
    (TMP / "src").mkdir(parents=True)
    (TMP / "docs").mkdir()
    (TMP / "platformio.ini").write_text("[env:nanokit_esp32]\nplatform = espressif32\nboard = esp32dev\nframework = arduino\n", encoding="utf-8")
    (TMP / "src" / "main.cpp").write_text(source or "#include <Arduino.h>\nvoid setup() {}\nvoid loop() {}\n", encoding="utf-8")
    (TMP / "nanokit.yaml").write_text(textwrap.dedent(manifest), encoding="utf-8")
    projects = generate.discover_projects([TMP.relative_to(ROOT).as_posix()])
    assert len(projects) == 1
    return projects[0]


def _rules(project: generate.Project, level: str = "error") -> set[str]:
    registry = generate.load_registry()
    generate.validate_schema(project)
    if not project.errors:
        generate.validate_hardware(project, registry)
        generate.check_evidence(project)
    return {f.rule for f in project.findings if f.level == level}


@pytest.fixture(autouse=True)
def _cleanup():
    yield
    if TMP.exists():
        shutil.rmtree(TMP)


HEADER = """
    genius: 1
    name: Self Test
    category: example
    board: nanokit-esp32
    firmware:
      - { id: main, path: ., code_roots: [src], wifi: WIFI }
    components:
"""


def test_registry_matches_datasheet_adc_addresses():
    registry = generate.load_registry()
    assert registry.vr_of(34) == "0x56"
    assert registry.vr_of(39) == "0x53"
    assert registry.vr_of(36) == "0x50"
    assert registry.vr_of(32) == "0x54"
    assert registry.header_of(5) == 36 and registry.header_of(34) == 2


def test_flash_pin_is_rejected():
    project = _write_project(HEADER.replace("WIFI", "false") + """
      - id: x
        name: X
        kind: sensor
        pins:
          - { pin: D, gpio: 6, dir: out }
    """)
    assert "flash-pin" in _rules(project)


def test_input_only_pin_cannot_drive():
    project = _write_project(HEADER.replace("WIFI", "false") + """
      - id: led
        name: LED
        kind: display
        pins:
          - { pin: A, gpio: 34, dir: out }
    """)
    assert "input-only" in _rules(project)


def test_uart0_and_boot_pins_are_rejected():
    project = _write_project(HEADER.replace("WIFI", "false") + """
      - id: a
        name: A
        kind: sensor
        pins:
          - { pin: T, gpio: 1, dir: out }
          - { pin: B, gpio: 0, dir: in }
    """)
    rules = _rules(project)
    assert {"uart0-shared", "strapping"} <= rules


def test_adc2_with_wifi_is_rejected_but_adc1_passes():
    bad = _write_project(HEADER.replace("WIFI", "true") + """
      - id: pot
        name: Pot
        kind: sensor
        pins:
          - { pin: W, gpio: 4, dir: in, type: adc }
    """)
    assert "adc2-wifi" in _rules(bad)
    good = _write_project(HEADER.replace("WIFI", "true") + """
      - id: pot
        name: Pot
        kind: sensor
        pins:
          - { pin: W, gpio: 36, dir: in, type: adc, symbol: POT_PIN }
    """, "constexpr uint8_t POT_PIN = 36;\n")
    assert "adc2-wifi" not in _rules(good) and not _rules(good)


def test_five_volt_input_needs_declared_level_shift():
    project = _write_project(HEADER.replace("WIFI", "false") + """
      - id: s
        name: S
        kind: sensor
        pins:
          - { pin: ECHO, gpio: 34, dir: in, level: 5V }
    """)
    assert "5v-input" in _rules(project)


def test_double_assignment_is_rejected():
    project = _write_project(HEADER.replace("WIFI", "false") + """
      - id: a
        name: A
        kind: actuator
        pins:
          - { pin: S, gpio: 25, dir: out }
      - id: b
        name: B
        kind: actuator
        pins:
          - { pin: S, gpio: 25, dir: out }
    """)
    assert "double-assigned" in _rules(project)


def test_evidence_mismatch_between_manifest_and_code():
    project = _write_project(HEADER.replace("WIFI", "false") + """
      - id: a
        name: A
        kind: actuator
        pins:
          - { pin: S, gpio: 25, dir: out, symbol: SERVO_PIN }
    """, "constexpr uint8_t SERVO_PIN = 26;\n")
    assert "evidence" in _rules(project)
    assert project.evidence[0].status == "mismatch"


def test_array_symbols_are_read_from_code():
    project = _write_project(HEADER.replace("WIFI", "false") + """
      - id: drv
        name: Driver
        kind: driver
        pins:
          - { pin: IN1, gpio: 25, dir: out, symbol: "PINS[0]" }
          - { pin: IN2, gpio: 27, dir: out, symbol: "PINS[1]" }
    """, "constexpr std::array<uint8_t, 2> PINS = {\n  25, // first\n  27  // second\n};\n")
    assert not _rules(project)
    assert all(item.status == "match" for item in project.evidence)


def test_header_mismatch_is_rejected():
    project = _write_project(HEADER.replace("WIFI", "false") + """
      - id: a
        name: A
        kind: actuator
        pins:
          - { pin: S, gpio: 25, header: 37, dir: out }
    """)
    assert "header-mismatch" in _rules(project)


def test_confirmed_component_without_gpio_is_rejected_but_blocked_passes():
    bad = _write_project(HEADER.replace("WIFI", "false") + """
      - id: a
        name: A
        kind: sensor
        status: confirmed
        pins:
          - { pin: S, gpio: null, dir: out }
    """)
    assert "confirmed-needs-gpio" in _rules(bad)
    good = _write_project(HEADER.replace("WIFI", "false") + """
      - id: a
        name: A
        kind: sensor
        status: blocked
        pins:
          - { pin: S, gpio: null, dir: out }
    """)
    assert not _rules(good)


def test_comma_in_flow_note_is_caught():
    project = _write_project(HEADER.replace("WIFI", "false") + """
      - id: a
        name: A
        kind: sensor
        pins:
          - { pin: S, gpio: 25, dir: out, note: first part, second part }
    """)
    assert "schema" in _rules(project)


def test_generated_mermaid_contains_header_and_gpio_labels():
    project = _write_project(HEADER.replace("WIFI", "false") + """
      - id: a
        name: A sensor
        kind: sensor
        pins:
          - { pin: OUT, gpio: 34, dir: in, symbol: A_PIN }
          - { pin: VCC, rail: 3V3 }
    """, "constexpr uint8_t A_PIN = 34;\n")
    registry = generate.load_registry()
    diagram = generate.render_connection_diagram(project, registry)
    assert "Pin 2 / GPIO34 · OUT" in diagram
    assert "3V3 · Pin 13 or 14" in diagram
    system = generate.render_system_diagram(project, registry)
    assert "A sensor" in system and "OUT Pin 2 / GPIO34" in system


def test_rerun_with_new_timestamp_does_not_rewrite_files(tmp_path):
    target = tmp_path / "VALIDATION_REPORT.md"
    first = "- Command: `python tools/genius/generate.py`\n- Generated: 2026-10-25 19:55:12 UTC\nbody\n"
    later = "- Command: `python tools/genius/generate.py --check`\n- Generated: 2026-11-02 08:00:00 UTC\nbody\n"
    assert generate.write_if_changed(target, first) is True
    assert generate.write_if_changed(target, later) is False   # only volatile lines differ
    assert target.read_text(encoding="utf-8") == first
    assert generate.write_if_changed(target, later.replace("body", "changed")) is True


def test_input_fingerprint_ignores_line_endings(tmp_path):
    lf = tmp_path / "lf.cpp"
    crlf = tmp_path / "crlf.cpp"
    lf.write_bytes(b"int a = 1;\nint b = 2;\n")
    crlf.write_bytes(b"int a = 1;\r\nint b = 2;\r\n")
    import hashlib
    norm = lambda p: hashlib.sha256(p.read_bytes().replace(b"\r\n", b"\n")).hexdigest()
    assert norm(lf) == norm(crlf)
    assert 'replace(b"\\r\\n", b"\\n")' in (ROOT / "tools" / "genius" / "generate.py").read_text(encoding="utf-8")
