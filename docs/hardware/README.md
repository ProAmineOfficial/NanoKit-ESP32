# Hardware

NanoKit hardware notes, board documentation, and pinout references.

## References

| Document | Purpose |
|---|---|
| [NanoKit Integrated ESP32 Pinout Reference](nanokit-integrated-esp32-pinout.md) | Official repository pinout table, GPIO restrictions, wiring template, and recommended pin choices. |

## Documentation Rule

Every example or application that uses external wiring must document:

- NanoKit pin number
- ESP32 GPIO number
- Component pin
- Signal purpose
- Electrical notes
- Warnings and restrictions

Use this mapping format:

```text
NanoKit Pin -> GPIO -> Component Pin -> Description
```
