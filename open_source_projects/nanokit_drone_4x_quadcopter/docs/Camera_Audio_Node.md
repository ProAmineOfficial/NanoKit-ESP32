# Camera and Audio Node - NanoKit Drone 4X

**Developed by Amine Saoud ibn al-Bashir.**

## Boundary

The payload is a separate ESP32 node. It may provide images, recording, audio, lighting measurements, and camera-servo control, but it has no motor authority and cannot bypass the NanoKit flight-controller safety state.

## Hard Requirements

- NanoKit #2 must physically contain and report at least 8 MB PSRAM before OV2640, audio, and microSD services are enabled together.
- Camera, I2S audio, microSD, servo, and BH1750 pins must come from a verified schematic or bench-tested wiring table.
- The servo must use a separate suitable power rail with common ground; it must not draw power from an ESP32 GPIO.
- Media loss or payload reset must not affect the 250 Hz flight task or arm state.

## Current Firmware Behaviour

All payload feature flags in `camera_node/include/camera_node_config.h` are `0`. The node joins the NanoKit Wi-Fi network and exposes status endpoints, but each unavailable service returns a clear disabled reason. It does not fabricate a stream, recording state, or audio level.

At startup the node checks `psramFound()` and `ESP.getPsramSize()`. Camera initialization remains blocked when less than 8 MB is detected, even if a feature flag is accidentally enabled.

## Integration Sequence

1. Verify PSRAM size using serial output.
2. Confirm the complete OV2640 pin map and enable camera-only capture at a conservative frame size.
3. Measure stability and memory before adding microSD.
4. Confirm I2S pins and enable microphone capture without playback.
5. Add MAX98357A playback and confirm audio tasks do not starve camera service.
6. Add servo control with an independent power test.
7. Add recording only after storage current draw and write latency are measured.

Each stage requires a feature-specific failure test and a documented rollback path.
