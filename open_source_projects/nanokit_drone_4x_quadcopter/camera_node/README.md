# NanoKit Drone 4X Camera + Audio Node

**Developed by Amine Saoud ibn al-Bashir.**

This is the second NanoKit ESP32 firmware project. It joins the flight controller's
`NanoKit-Drone-4X` SoftAP and exposes camera/audio/storage control endpoints without
ever receiving motor authority.

All peripheral features are disabled in `include/camera_node_config.h` until the
pin map is verified. Camera startup also requires a runtime-confirmed 8 MB PSRAM
device. The status API remains usable while hardware features are disabled.

```powershell
pio run
pio run -t upload
pio device monitor
```

See [Camera_Audio_Node.md](../docs/Camera_Audio_Node.md) for the validation gates.
