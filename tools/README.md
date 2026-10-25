# Tools

**Developed by Amine Saoud ibn al-Bashir.**

| Tool | Purpose |
|---|---|
| `genius/generate.py` | Validates every project `nanokit.yaml` against `registry/nanokit-esp32.yaml` and the firmware source, then regenerates connection/system diagrams, pin tables, and `.genius/` reports. |
| `genius/tests/` | Self-tests for the validation rules (`python -m pytest tools/genius/tests -q`). |
| `update_repository_index.py` | Regenerates `REPOSITORY_TREE.md`, `REPOSITORY_INDEX.md`, and the per-area catalogs from the manifests. |
| `sync_with_github.ps1` | Windows helper for pushing local work. |

Run order after any hardware or project change:

```bash
pip install pyyaml
python tools/genius/generate.py
python tools/update_repository_index.py
```
