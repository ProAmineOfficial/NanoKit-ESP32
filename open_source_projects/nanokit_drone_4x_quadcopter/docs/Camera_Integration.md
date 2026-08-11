# Camera Integration - NanoKit Drone 4X

**Developed by Amine Saoud ibn al-Bashir.**

## Architecture Decision

Camera and audio functions run on a second ESP32 node. The NanoKit flight controller remains the only motor authority. Payload HTTP requests cannot arm, change throttle, select a flight mode, or write an ESC signal.

## Current Hardware Status

The final OV2640 pin map is not confirmed, so no camera pins are listed or enabled. Previous reference wiring is intentionally not treated as a NanoKit wiring specification.

NanoKit #2 must report at least 8 MB PSRAM at runtime before camera startup. This is a hard software gate because an ESP32 module without PSRAM is not suitable for the proposed OV2640, audio, and microSD workload.

## Network Boundary

The payload node joins `NanoKit-Drone-4X` as a Wi-Fi station and announces `nanokit-camera.local` when mDNS is available. Status endpoints report which services are compiled, verified, or unavailable. The Flight Deck may show a payload stream and call payload controls, but flight-command safety remains independent.

## Required Verification

1. Confirm the exact ESP32 module and 8 MB PSRAM.
2. Confirm every OV2640 signal from a schematic or measured board wiring.
3. Test camera capture alone before adding audio or storage.
4. Confirm I2S microphone and amplifier pins and their clock compatibility.
5. Confirm microSD SPI pins, supply current, and worst-case write latency.
6. Power the servo from an appropriate separate rail with common ground.

See [Camera and Audio Node](Camera_Audio_Node.md) for the staged activation sequence.
