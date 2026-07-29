# NanoKit Drone 4X Flight Deck

**Developed by Amine Saoud ibn al-Bashir.**

This original cockpit-style controller uses standard browser Web Bluetooth to connect directly to the NanoKit ESP32 BLE service. It supplies throttle, momentary roll/pitch/yaw commands, arm/disarm, IMU calibration, emergency stop, and telemetry display. A central camera viewport independently displays an MJPEG/image URL from the separate Arducam Mega Wi-Fi camera node.

## Run Locally

From this directory, start a localhost server:

```powershell
python -m http.server 8080
```

Open `http://localhost:8080` in Chrome or Edge. Web Bluetooth requires a supported Chromium-based browser and a secure context such as localhost. It does not work with an HC-05 Classic Bluetooth SPP module.

Keep all propellers removed whenever the controller is used for development or testing.

## Camera Viewport

Select the settings button in the Flight Deck, enter the private Wi-Fi camera endpoint, and select **Load Camera**. Camera loss never affects the BLE flight link or the firmware failsafe. See [Camera Integration](../docs/Camera_Integration.md).
