# Arducam Mega Camera Node

**Developed by Amine Saoud ibn al-Bashir.**

This directory defines the boundary for the optional front-camera subsystem. Use a dedicated ESP32 or second NanoKit with an Arducam Mega 5 MP SPI camera, then publish its image/MJPEG endpoint over a private Wi-Fi network.

The production flight-controller firmware deliberately does not include a camera library or Wi-Fi image server. It currently builds with a high flash footprint due to BLE and safety logic; separating the camera protects the control-loop timing and means video failure cannot affect motor failsafes.

Follow [Camera Integration](../docs/Camera_Integration.md) for wiring, upstream library links, endpoint convention, and safety constraints.
