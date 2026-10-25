#!/usr/bin/env python3
# Developed by Amine Saoud ibn al-Bashir.
"""Generate repository tree and project catalogs for NanoKit-ESP32."""

from __future__ import annotations

import json
import os
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
MANIFEST_NAME = "nanokit.yaml"  # a folder is a project only when it carries a Genius hardware manifest
ROOT_README_TREE_START = "<!-- GENERATED_REPOSITORY_TREE:START -->"
ROOT_README_TREE_END = "<!-- GENERATED_REPOSITORY_TREE:END -->"

SKIP_DIRS = {
    ".git",
    ".pio",
    ".pioenvs",
    ".piolibdeps",
    ".pytest_cache",
    ".ruff_cache",
    ".mypy_cache",
    "__pycache__",
    "build",
    "dist",
    "htmlcov",
    "node_modules",
    "venv",
    ".venv",
    "env",
}

SKIP_FILES = {
    ".DS_Store",
    "Thumbs.db",
}

TOP_LEVEL_AREAS = [
    ("examples_on_platformio", "Focused PlatformIO examples"),
    ("applications_on_platformio", "Complete multi-part applications"),
    ("open_source_projects", "Original open-source-inspired NanoKit projects"),
    ("graduation_projects", "Academic and thesis projects"),
    ("tutorials", "Learning paths and tutorials"),
    ("articles", "Technical articles"),
    ("docs", "Core documentation"),
    ("simulator", "Simulation resources"),
    ("templates", "Reusable templates"),
    ("assets", "Shared assets"),
    ("registry", "Board registry (pin data for Genius validation)"),
    ("tools", "Repository automation"),
]


def read_title(readme: Path, fallback: str) -> str:
    if not readme.exists():
        return fallback.replace("_", " ").replace("-", " ").title()
    for line in readme.read_text(encoding="utf-8").splitlines():
        if line.startswith("# "):
            return line[2:].strip()
    return fallback.replace("_", " ").replace("-", " ").title()


def has_file(path: Path, relative: str) -> str:
    return "yes" if (path / relative).exists() else "no"


def list_project_dirs(area: str) -> list[Path]:
    """Projects are folders that carry `nanokit.yaml`; category folders such as
    `open_source_projects/templates/` are listed separately by `list_category_dirs`."""
    root = ROOT / area
    if not root.exists():
        return []
    projects = {manifest.parent for manifest in root.rglob(MANIFEST_NAME) if not (set(manifest.parts) & SKIP_DIRS)}
    return sorted(projects, key=lambda item: item.name.lower())


def list_category_dirs(area: str) -> list[Path]:
    """Direct sub-folders that are not projects (categories, templates, placeholders)."""
    root = ROOT / area
    if not root.exists():
        return []
    projects = {p.resolve() for p in list_project_dirs(area)}
    return sorted(
        [item for item in root.iterdir() if item.is_dir() and item.name not in SKIP_DIRS and item.resolve() not in projects],
        key=lambda item: item.name.lower(),
    )


def read_manifest(project: Path) -> dict:
    """Read the small set of manifest fields the catalogs need without requiring PyYAML."""
    info = {"status": "-", "firmware": []}
    manifest = project / MANIFEST_NAME
    if not manifest.exists():
        return info
    for line in manifest.read_text(encoding="utf-8").splitlines():
        stripped = line.strip()
        if stripped.startswith("status:") and not line.startswith(" "):
            info["status"] = stripped.split(":", 1)[1].strip()
        if stripped.startswith("path:") or stripped.startswith("- { id:"):
            for part in stripped.strip("-{} ").split(","):
                key, _, value = part.partition(":")
                if key.strip() == "path":
                    info["firmware"].append(value.strip())
    return info


def genius_result(project: Path) -> str:
    report = project / ".genius" / "project.json"
    if not report.exists():
        return "not run"
    try:
        data = json.loads(report.read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return "unreadable"
    result = str(data.get("result", "?"))
    errors = sum(1 for f in data.get("findings", []) if f.get("level") == "error")
    warnings = sum(1 for f in data.get("findings", []) if f.get("level") == "warning")
    return f"{result.title()} ({errors} err, {warnings} warn)"


def firmware_cell(project: Path, info: dict) -> str:
    paths = info["firmware"] or ["."]
    found = [path for path in paths if (project / path / "platformio.ini").exists()]
    if not found:
        return "no"
    return ", ".join("`" + (p if p != "." else "platformio.ini") + "`" for p in found)


REPOSITORY_NAME = "NanoKit-ESP32"  # fixed title: the clone folder may be named differently on each machine


def build_tree(root: Path) -> str:
    lines = [REPOSITORY_NAME + "/"]

    def walk(directory: Path, prefix: str = "") -> None:
        entries = [
            item
            for item in sorted(directory.iterdir(), key=lambda path: (path.is_file(), path.name.lower()))
            if item.name not in SKIP_FILES and not (item.is_dir() and item.name in SKIP_DIRS)
        ]
        for index, item in enumerate(entries):
            connector = "`-- " if index == len(entries) - 1 else "|-- "
            lines.append(f"{prefix}{connector}{item.name}{'/' if item.is_dir() else ''}")
            if item.is_dir():
                extension = "    " if index == len(entries) - 1 else "|   "
                walk(item, prefix + extension)

    walk(root)
    return "\n".join(lines)


def write_repository_tree() -> None:
    body = f"""# NanoKit-ESP32 Repository Tree

This file is generated by `tools/update_repository_index.py`.

**Developed by Amine Saoud ibn al-Bashir.**

Run the generator after adding, moving, or deleting examples, applications, documentation, or open-source projects.

```text
{build_tree(ROOT)}
```
"""
    (ROOT / "REPOSITORY_TREE.md").write_text(body, encoding="utf-8", newline="\n")


def write_root_readme_tree() -> None:
    """Replace only the generated tree block embedded in the root README."""
    readme = ROOT / "README.md"
    content = readme.read_text(encoding="utf-8")
    start_index = content.find(ROOT_README_TREE_START)
    end_index = content.find(ROOT_README_TREE_END)
    if start_index < 0 or end_index < 0 or end_index <= start_index:
        raise RuntimeError("README.md is missing its generated repository tree markers.")

    generated_block = (
        f"{ROOT_README_TREE_START}\n"
        f"```text\n{build_tree(ROOT)}\n```\n"
        f"{ROOT_README_TREE_END}"
    )
    updated = (
        content[:start_index]
        + generated_block
        + content[end_index + len(ROOT_README_TREE_END):]
    )
    readme.write_text(updated, encoding="utf-8", newline="\n")


def rel_link(project: Path, area: str) -> str:
    return project.relative_to(ROOT / area).as_posix()


def write_examples_catalog() -> None:
    rows = []
    for project in list_project_dirs("examples_on_platformio"):
        title = read_title(project / "README.md", project.name)
        info = read_manifest(project)
        rows.append(
            "| "
            + " | ".join(
                [
                    f"[{title}]({rel_link(project, 'examples_on_platformio')}/README.md)",
                    info["status"],
                    "yes" if (project / "platformio.ini").exists() else "no",
                    has_file(project, "src/main.cpp"),
                    has_file(project, "docs/Wiring.md"),
                    has_file(project, "docs/Implementation_Guide.md"),
                    has_file(project, "images/connection-diagram.md"),
                    genius_result(project),
                ]
            )
            + " |"
        )

    table = "\n".join(rows) if rows else "| No examples yet | - | no | no | no | no | no | - |"
    body = f"""# Examples on PlatformIO

Focused NanoKit Integrated ESP32 examples. Each example should remain small, buildable, documented, and aligned with `docs/software/platformio-example-standard.md`.

This catalog is generated by `tools/update_repository_index.py`. A folder is listed when it carries a `nanokit.yaml` hardware manifest; the Genius column comes from `.genius/project.json` written by `tools/genius/generate.py`.

**Developed by Amine Saoud ibn al-Bashir.**

| Example | Status | PlatformIO | Source | Wiring | Implementation | Diagrams | Genius validation |
|---|---|---|---|---|---|---|---|
{table}

## Maintenance

After adding a new example, run:

```bash
python tools/genius/generate.py
python tools/update_repository_index.py
```
"""
    (ROOT / "examples_on_platformio" / "README.md").write_text(body, encoding="utf-8", newline="\n")


def write_area_catalog(area: str, title: str, description: str) -> None:
    rows = []
    for project in list_project_dirs(area):
        project_title = read_title(project / "README.md", project.name)
        info = read_manifest(project)
        rows.append(
            "| "
            + " | ".join(
                [
                    f"[{project_title}]({rel_link(project, area)}/README.md)",
                    info["status"],
                    firmware_cell(project, info),
                    "yes" if (project / "docs").exists() else "no",
                    has_file(project, "images/connection-diagram.md"),
                    genius_result(project),
                ]
            )
            + " |"
        )
    categories = [f"- [{read_title(folder / 'README.md', folder.name)}]({folder.name}/README.md)" for folder in list_category_dirs(area)]

    table = "\n".join(rows) if rows else "| No projects yet | - | no | no | no | - |"
    category_block = "\n".join(categories) if categories else "- none"
    body = f"""# {title}

{description}

This catalog is generated by `tools/update_repository_index.py`. A folder is a project when it carries a `nanokit.yaml` hardware manifest; the Genius column comes from `.genius/project.json` written by `tools/genius/generate.py`.

**Developed by Amine Saoud ibn al-Bashir.**

## Projects

| Project | Status | Firmware | Docs | Diagrams | Genius validation |
|---|---|---|---|---|---|
{table}

## Categories and Templates

Folders without a manifest are categories, templates, or reserved workspaces, not projects:

{category_block}
"""
    (ROOT / area / "README.md").write_text(body, encoding="utf-8", newline="\n")


PROJECT_AREAS = {"examples_on_platformio", "applications_on_platformio", "open_source_projects", "graduation_projects"}


def write_root_index() -> None:
    rows = []
    for area, purpose in TOP_LEVEL_AREAS:
        path = ROOT / area
        folders = len([item for item in path.iterdir() if item.is_dir() and item.name not in SKIP_DIRS]) if path.exists() else 0
        projects = len(list_project_dirs(area)) if area in PROJECT_AREAS else "-"
        rows.append(f"| [{area}]({area}/README.md) | {purpose} | {projects} | {folders} |")
    table = "\n".join(rows)
    body = f"""# NanoKit-ESP32 Index

This file is generated by `tools/update_repository_index.py`.

**Developed by Amine Saoud ibn al-Bashir.**

| Area | Purpose | Projects (with `nanokit.yaml`) | Direct Folders |
|---|---|---:|---:|
{table}

See `REPOSITORY_TREE.md` for the full generated tree and `registry/nanokit-esp32.yaml` for the board registry that every project manifest is validated against.
"""
    (ROOT / "REPOSITORY_INDEX.md").write_text(body, encoding="utf-8", newline="\n")


def main() -> None:
    os.chdir(ROOT)
    write_repository_tree()
    write_root_readme_tree()
    write_root_index()
    write_examples_catalog()
    write_area_catalog(
        "open_source_projects",
        "Open Source Projects",
        "Original NanoKit-oriented projects inspired by open-source engineering practices. Do not copy external project code.",
    )
    write_area_catalog(
        "applications_on_platformio",
        "Applications on PlatformIO",
        "Complete NanoKit Integrated ESP32 applications with firmware, documentation, media, and reusable assets.",
    )
    write_area_catalog(
        "graduation_projects",
        "Graduation Projects",
        "Academic project workspaces and templates for NanoKit-ESP32: bachelor, master, engineering, institute, capstone, and university-thesis work.",
    )
    print("Updated REPOSITORY_TREE.md, REPOSITORY_INDEX.md, and generated catalogs.")


if __name__ == "__main__":
    main()
