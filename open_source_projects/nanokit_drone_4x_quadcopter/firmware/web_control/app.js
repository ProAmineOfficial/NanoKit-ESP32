/**
 * Project: NanoKit Drone 4X
 * Component: Flight Deck Web Interface Logic
 * File: app.js
 *
 * This file is part of the NanoKit Drone 4X open-source project.
 *
 * Description:
 * Handles user interactions, WebSocket communication, command transmission,
 * real-time telemetry updates, interface state management, and camera controls
 * for the NanoKit Flight Deck.
 *
 * Developed by Amine Saoud ibn al-Bashir
 * Pro_Amine LLC
 * Official website: https://proamine.tech
 *
 * Repository: NanoKit-ESP32
 * License: Apache License 2.0
 */

"use strict"; // NanoKit Drone 4X Wi-Fi Flight Deck. Developed by Amine Saoud ibn al-Bashir. Only validated telemetry is rendered. Missing sensors remain visibly offline.

const PROTOCOL_VERSION = "3"; // Keep browser packets synchronized with firmware protocol version 3.
const COMMAND_PERIOD_MS = 100; // Send commands often enough to remain inside the firmware's 600 ms timeout.
const ARM_HOLD_MS = 1500; // Require an intentional hold before requesting motor authority.
const DEFAULT_FLIGHT_HOST = ["127.0.0.1", "localhost"].includes(location.hostname) // Local development pages still connect to the NanoKit SoftAP host by default.
  ? "192.168.4.1" // Continue the current Flight Deck JavaScript declaration or operation.
  : location.hostname || "192.168.4.1"; // Execute this JavaScript statement for the interface.
const DEFAULT_CAMERA_URL = "http://nanokit-camera.local/stream"; // Camera defaults use the separate payload node's mDNS hostname.
const DEFAULT_CAMERA_API_URL = "http://nanokit-camera.local";
const SETTINGS_KEY = "nanokit-drone-4x-flight-deck-settings-v3"; // Versioned keys prevent older local settings from corrupting this interface version.
const MISSION_KEY = "nanokit-drone-4x-local-mission"; // Declare this interface state, element reference, or fixed configuration value.

const clamp = (value, minimum, maximum) => Math.max(minimum, Math.min(maximum, value)); // Clamp untrusted pointer and telemetry numbers to their valid interface ranges.
const byId = (id) => document.getElementById(id); // Centralize DOM lookup so the element map below stays concise and readable.

const elements = { // Cache every interface element once instead of querying the DOM during control updates.
  flightDeck: byId("flightDeck"), // Top-level link, safety, status, and settings elements.
  linkDot: byId("linkDot"), // Execute or continue this browser control operation.
  linkText: byId("linkText"), // Execute or continue this browser control operation.
  gnssTop: byId("gnssTop"), // Execute or continue this browser control operation.
  armTop: byId("armTop"), // Execute or continue this browser control operation.
  batteryTop: byId("batteryTop"), // Execute or continue this browser control operation.
  connectButton: byId("connectButton"), // Execute or continue this browser control operation.
  settingsButton: byId("settingsButton"), // Execute or continue this browser control operation.
  alertBar: byId("alertBar"), // Execute or continue this browser control operation.
  alertTitle: byId("alertTitle"), // Execute or continue this browser control operation.
  alertMessage: byId("alertMessage"), // Execute or continue this browser control operation.
  dismissAlert: byId("dismissAlert"), // Execute or continue this browser control operation.
  videoStage: byId("videoStage"), // Camera stage, HUD, live telemetry, and endpoint elements.
  offlineReference: byId("offlineReference"), // Execute or continue this browser control operation.
  cameraFeed: byId("cameraFeed"), // Execute or continue this browser control operation.
  cameraSignalDot: byId("cameraSignalDot"), // Execute or continue this browser control operation.
  cameraSignalText: byId("cameraSignalText"), // Execute or continue this browser control operation.
  cameraEndpointLabel: byId("cameraEndpointLabel"), // Execute or continue this browser control operation.
  latencyValue: byId("latencyValue"), // Execute or continue this browser control operation.
  flightTimeValue: byId("flightTimeValue"), // Execute or continue this browser control operation.
  headingValue: byId("headingValue"), // Execute or continue this browser control operation.
  rollValue: byId("rollValue"), // Execute or continue this browser control operation.
  pitchValue: byId("pitchValue"), // Execute or continue this browser control operation.
  altitudeValue: byId("altitudeValue"), // Execute or continue this browser control operation.
  speedValue: byId("speedValue"), // Execute or continue this browser control operation.
  voltageValue: byId("voltageValue"), // Execute or continue this browser control operation.
  currentValue: byId("currentValue"), // Execute or continue this browser control operation.
  satelliteValue: byId("satelliteValue"), // Execute or continue this browser control operation.
  commandAgeValue: byId("commandAgeValue"), // Execute or continue this browser control operation.
  safetyState: byId("safetyState"), // Flight safety controls and the authoritative state label.
  armHoldButton: byId("armHoldButton"), // Execute or continue this browser control operation.
  disarmButton: byId("disarmButton"), // Execute or continue this browser control operation.
  returnHomeButton: byId("returnHomeButton"), // Execute or continue this browser control operation.
  calibrateImuButton: byId("calibrateImuButton"), // Execute or continue this browser control operation.
  payloadState: byId("payloadState"), // Separate payload-node controls never receive motor authority.
  snapshotButton: byId("snapshotButton"), // Execute or continue this browser control operation.
  recordButton: byId("recordButton"), // Execute or continue this browser control operation.
  audioButton: byId("audioButton"), // Execute or continue this browser control operation.
  speakerButton: byId("speakerButton"), // Execute or continue this browser control operation.
  cameraTilt: byId("cameraTilt"), // Execute or continue this browser control operation.
  cameraTiltValue: byId("cameraTiltValue"), // Execute or continue this browser control operation.
  fullscreenButton: byId("fullscreenButton"), // Execute or continue this browser control operation.
  leftStick: byId("leftStick"), // Virtual pilot sticks and their numeric readouts.
  leftStickKnob: byId("leftStickKnob"), // Execute or continue this browser control operation.
  leftStickReadout: byId("leftStickReadout"), // Execute or continue this browser control operation.
  rightStick: byId("rightStick"), // Execute or continue this browser control operation.
  rightStickKnob: byId("rightStickKnob"), // Execute or continue this browser control operation.
  rightStickReadout: byId("rightStickReadout"), // Execute or continue this browser control operation.
  streamMode: byId("streamMode"), // Execute or continue this browser control operation.
  frameInfo: byId("frameInfo"), // Execute or continue this browser control operation.
  coordinateValue: byId("coordinateValue"), // Execute or continue this browser control operation.
  sensorGateValue: byId("sensorGateValue"), // Execute or continue this browser control operation.
  missionWorkspace: byId("missionWorkspace"), // Local mission-planning elements remain locked from upload until navigation is verified.
  missionMap: byId("missionMap"), // Execute or continue this browser control operation.
  missionPath: byId("missionPath"), // Execute or continue this browser control operation.
  waypointList: byId("waypointList"), // Execute or continue this browser control operation.
  saveMissionButton: byId("saveMissionButton"), // Execute or continue this browser control operation.
  clearMissionButton: byId("clearMissionButton"), // Execute or continue this browser control operation.
  uploadMissionButton: byId("uploadMissionButton"), // Execute or continue this browser control operation.
  throttleStrip: byId("throttleStrip"), // Lower telemetry strip and event log elements.
  throttleMeter: byId("throttleMeter"), // Execute or continue this browser control operation.
  powerValue: byId("powerValue"), // Execute or continue this browser control operation.
  modeValue: byId("modeValue"), // Execute or continue this browser control operation.
  flowValue: byId("flowValue"), // Execute or continue this browser control operation.
  obstacleValue: byId("obstacleValue"), // Execute or continue this browser control operation.
  temperatureValue: byId("temperatureValue"), // Execute or continue this browser control operation.
  humidityValue: byId("humidityValue"), // Execute or continue this browser control operation.
  eventLog: byId("eventLog"), // Execute or continue this browser control operation.
  emergencyButton: byId("emergencyButton"), // Execute or continue this browser control operation.
  settingsDialog: byId("settingsDialog"), // Connection and camera endpoints are edited through this modal settings form.
  settingsForm: byId("settingsForm"), // Execute or continue this browser control operation.
  closeSettings: byId("closeSettings"), // Execute or continue this browser control operation.
  cancelSettings: byId("cancelSettings"), // Execute or continue this browser control operation.
  flightHost: byId("flightHost"), // Execute or continue this browser control operation.
  cameraUrl: byId("cameraUrl"), // Execute or continue this browser control operation.
  cameraApiUrl: byId("cameraApiUrl"), // Execute or continue this browser control operation.
}; // Close the current JavaScript scope or object.

const savedSettings = (() => { // Load saved endpoints defensively because localStorage may contain invalid JSON.
  try { // Attempt this operation while allowing controlled failure handling.
    return JSON.parse(localStorage.getItem(SETTINGS_KEY) || "{}"); // Return this computed result to the caller.
  } catch (_error) { // Close the current JavaScript scope or object.
    return {}; // Return this computed result to the caller.
  } // Close the current JavaScript scope or object.
})(); // Close the current JavaScript scope or object.

const state = { // Keep all mutable interface and protocol state in one explicit object.
  socket: null, // WebSocket lifecycle, timers, packet sequences, and measured latency.
  connected: false, // Continue the current Flight Deck JavaScript declaration or operation.
  connecting: false, // Continue the current Flight Deck JavaScript declaration or operation.
  commandTimer: null, // Continue the current Flight Deck JavaScript declaration or operation.
  uiTimer: null, // Continue the current Flight Deck JavaScript declaration or operation.
  payloadTimer: null, // Continue the current Flight Deck JavaScript declaration or operation.
  nextSequence: 1, // Continue the current Flight Deck JavaScript declaration or operation.
  pendingCommands: new Map(), // Execute or continue this browser control operation.
  latencyMs: null, // Continue the current Flight Deck JavaScript declaration or operation.
  lastTelemetryAt: 0, // Continue the current Flight Deck JavaScript declaration or operation.
  flightHost: savedSettings.flightHost || DEFAULT_FLIGHT_HOST, // Operator-configurable endpoints fall back to safe NanoKit defaults.
  cameraUrl: savedSettings.cameraUrl || DEFAULT_CAMERA_URL, // Continue the current Flight Deck JavaScript declaration or operation.
  cameraApiUrl: savedSettings.cameraApiUrl || DEFAULT_CAMERA_API_URL, // Continue the current Flight Deck JavaScript declaration or operation.
  cameraLive: false, // Camera-node readiness is reported independently for each payload function.
  payload: { // Open this JavaScript implementation block.
    cameraReady: false, // Continue the current Flight Deck JavaScript declaration or operation.
    audioCaptureReady: false, // Continue the current Flight Deck JavaScript declaration or operation.
    audioPlaybackReady: false, // Continue the current Flight Deck JavaScript declaration or operation.
    storageReady: false, // Continue the current Flight Deck JavaScript declaration or operation.
    psram8MB: false, // Continue the current Flight Deck JavaScript declaration or operation.
  }, // Close the current JavaScript scope or object.
  recording: false, // Local view and payload interaction state never grants flight authority.
  audioMonitoring: false, // Continue the current Flight Deck JavaScript declaration or operation.
  speakerActive: false, // Continue the current Flight Deck JavaScript declaration or operation.
  view: "standard", // Continue the current Flight Deck JavaScript declaration or operation.
  flightMode: "MANUAL", // Continue the current Flight Deck JavaScript declaration or operation.
  armRequested: false, // Arming and one-shot action timers control the command flags sent to firmware.
  armed: false, // Continue the current Flight Deck JavaScript declaration or operation.
  armedSince: 0, // Continue the current Flight Deck JavaScript declaration or operation.
  accumulatedFlightMs: 0, // Continue the current Flight Deck JavaScript declaration or operation.
  stopPulseUntil: 0, // Continue the current Flight Deck JavaScript declaration or operation.
  calibrationPulseUntil: 0, // Continue the current Flight Deck JavaScript declaration or operation.
  armHoldTimer: null, // Continue the current Flight Deck JavaScript declaration or operation.
  control: { throttle: 0, roll: 0, pitch: 0, yaw: 0 }, // Pilot commands use normalized throttle and engineering units for all axes.
  telemetry: {}, // The newest telemetry packet and its explicit validity gates are retained together.
  safety: { // Open this JavaScript implementation block.
    imuHealthy: false, // Continue the current Flight Deck JavaScript declaration or operation.
    imuCalibrated: false, // Continue the current Flight Deck JavaScript declaration or operation.
    attitudeValid: false, // Continue the current Flight Deck JavaScript declaration or operation.
    escConfirmed: false, // Continue the current Flight Deck JavaScript declaration or operation.
    gnssFix: false, // Continue the current Flight Deck JavaScript declaration or operation.
    barometerValid: false, // Continue the current Flight Deck JavaScript declaration or operation.
    environmentValid: false, // Continue the current Flight Deck JavaScript declaration or operation.
    powerValid: false, // Continue the current Flight Deck JavaScript declaration or operation.
    navigationReady: false, // Continue the current Flight Deck JavaScript declaration or operation.
  }, // Close the current JavaScript scope or object.
  waypoints: [], // Waypoints are local planning data until navigation hardware is validated.
}; // Close the current JavaScript scope or object.

function parsePacket(payload) { // Parse one semicolon-separated firmware packet into a key/value object.
  const fields = {}; // Declare this interface state, element reference, or fixed configuration value.
  String(payload).split(";").forEach((part) => { // Ignore malformed fragments so one bad field cannot break the full interface.
    const separator = part.indexOf("="); // Declare this interface state, element reference, or fixed configuration value.
    if (separator <= 0) return; // Evaluate this condition before continuing the interface update.
    fields[part.slice(0, separator).trim()] = part.slice(separator + 1).trim(); // Execute or continue this browser control operation.
  }); // Close the current JavaScript scope or object.
  return fields; // Return this computed result to the caller.
} // Close the current JavaScript scope or object.

function setEvent(message) { // Show the newest operator or protocol event in the persistent event log.
  elements.eventLog.textContent = message; // Update this value for the current Flight Deck state.
} // Close the current JavaScript scope or object.

function setAlert(level, title, message) { // Present a visible safety alert using one controlled severity style.
  elements.alertBar.hidden = false; // Update this value for the current Flight Deck state.
  elements.alertBar.classList.remove("warning", "critical", "good"); // Execute or continue this browser control operation.
  elements.alertBar.classList.add(level); // Execute or continue this browser control operation.
  elements.alertTitle.textContent = title; // Update this value for the current Flight Deck state.
  elements.alertMessage.textContent = message; // Update this value for the current Flight Deck state.
} // Close the current JavaScript scope or object.

function setStatusDot(element, level) { // Replace a status indicator's severity class without changing its element.
  element.classList.remove("good", "warning", "critical"); // Execute or continue this browser control operation.
  element.classList.add(level); // Execute or continue this browser control operation.
} // Close the current JavaScript scope or object.

function normalizeHost(value) { // Convert a host, URL, or address string into only the hostname used by WebSocket.
  const raw = String(value || "").trim(); // Declare this interface state, element reference, or fixed configuration value.
  if (!raw) return DEFAULT_FLIGHT_HOST; // Evaluate this condition before continuing the interface update.
  try { // Prefer standards-based URL parsing when the browser can understand the input.
    const parsed = new URL(raw.includes("://") ? raw : `http://${raw}`);
    return parsed.hostname || DEFAULT_FLIGHT_HOST; // Return this computed result to the caller.
  } catch (_error) { // Close the current JavaScript scope or object.
    return raw.replace(/^(https?|wss?):\/\//i, "").split("/")[0].split(":")[0]; // Fall back to simple prefix, path, and port removal for partial user input.
  } // Close the current JavaScript scope or object.
} // Close the current JavaScript scope or object.

function websocketUrl() { // Build the fixed unencrypted local WebSocket endpoint used by the NanoKit SoftAP.
  return `ws://${normalizeHost(state.flightHost)}:81/`;
} // Close the current JavaScript scope or object.

function setLinkState(connected, detail = "") { // Update connection state, controls, status indicators, and the operator event together.
  state.connected = connected; // Update this value for the current Flight Deck state.
  state.connecting = false; // Update this value for the current Flight Deck state.
  elements.linkText.textContent = connected ? "LINKED" : "OFFLINE"; // Update this value for the current Flight Deck state.
  setStatusDot(elements.linkDot, connected ? "good" : "critical"); // Execute or continue this browser control operation.
  elements.connectButton.textContent = connected ? "Disconnect" : "Connect"; // Update this value for the current Flight Deck state.
  elements.connectButton.disabled = false; // Update this value for the current Flight Deck state.
  setEvent(detail || (connected ? "WebSocket command link active" : "Flight link offline")); // Execute or continue this browser control operation.
  updateSafetyControls(); // Re-evaluate arming because connection state is one of its mandatory gates.
} // Close the current JavaScript scope or object.

function formatNumber(value, digits = 1) { // Format a finite numeric field or return null for missing and invalid telemetry.
  const number = Number(value); // Declare this interface state, element reference, or fixed configuration value.
  return Number.isFinite(number) ? number.toFixed(digits) : null; // Return this computed result to the caller.
} // Close the current JavaScript scope or object.

function fieldEnabled(fields, key) { // Treat only the protocol value "1" as an enabled hardware validity flag.
  return fields[key] === "1"; // Return this computed result to the caller.
} // Close the current JavaScript scope or object.

function updateSafetyControls() { // Enable actions only when the current link, pilot, and firmware safety gates allow them.
  const controlsCentered = Math.abs(state.control.roll) < 0.01 && // Centered attitude controls are required before the interface permits arming.
    Math.abs(state.control.pitch) < 0.01 && Math.abs(state.control.yaw) < 0.01; // Execute or continue this browser control operation.
  const canArm = state.connected && !state.armed && state.control.throttle === 0 && // Declare this interface state, element reference, or fixed configuration value.
    controlsCentered && state.safety.imuHealthy && state.safety.imuCalibrated && // Continue the current Flight Deck JavaScript declaration or operation.
    state.safety.attitudeValid && state.safety.escConfirmed; // Execute this JavaScript statement for the interface.

  elements.armHoldButton.disabled = !canArm; // Arm, disarm, calibration, return-home, and mission controls have independent gates.
  elements.armHoldButton.classList.toggle("armed", state.armed); // Execute or continue this browser control operation.
  elements.armHoldButton.querySelector("b").textContent = state.armed ? "SYSTEM ARMED" : "HOLD TO ARM"; // Execute or continue this browser control operation.
  elements.armHoldButton.querySelector("small").textContent = state.armed ? "Live control authority" : "1.5 seconds"; // Execute or continue this browser control operation.
  elements.disarmButton.disabled = !state.connected || !state.armed; // Update this value for the current Flight Deck state.
  elements.calibrateImuButton.disabled = !state.connected || !state.safety.imuHealthy || state.armed; // Update this value for the current Flight Deck state.
  elements.returnHomeButton.disabled = !state.connected || !state.armed || // Update this value for the current Flight Deck state.
    !state.safety.gnssFix || !state.safety.navigationReady; // Execute this JavaScript statement for the interface.
  elements.uploadMissionButton.disabled = !state.connected || !state.safety.navigationReady || // Update this value for the current Flight Deck state.
    state.waypoints.length === 0; // Update this value for the current Flight Deck state.

  const safetyName = state.telemetry.STATE || (state.connected ? "DISARMED" : "LOCKED"); // Firmware telemetry remains the authoritative visible safety state.
  elements.safetyState.textContent = safetyName; // Update this value for the current Flight Deck state.
  elements.armTop.textContent = state.armed ? "ARMED" : safetyName; // Update this value for the current Flight Deck state.
  elements.sensorGateValue.textContent = state.safety.imuHealthy // Update this value for the current Flight Deck state.
    ? (state.safety.imuCalibrated ? "IMU READY" : "CALIBRATION") // Execute or continue this browser control operation.
    : "IMU LOCK"; // Execute this JavaScript statement for the interface.
} // Close the current JavaScript scope or object.

function updateControlDisplay() { // Render pilot command values and virtual-stick positions without sending a packet.
  const throttlePercent = Math.round(state.control.throttle / 10); // Convert normalized 0-1000 throttle into a display percentage.
  elements.leftStickReadout.textContent = `T ${throttlePercent} / Y ${Math.round(state.control.yaw)}`; // Execute or continue this browser control operation.
  elements.rightStickReadout.textContent = `P ${Math.round(state.control.pitch)} / R ${Math.round(state.control.roll)}`; // Execute or continue this browser control operation.
  elements.throttleStrip.textContent = `${throttlePercent}%`; // Update this value for the current Flight Deck state.
  elements.throttleMeter.style.width = `${throttlePercent}%`; // Update this value for the current Flight Deck state.
  elements.leftStick.style.setProperty("--stick-x", `${50 + (state.control.yaw / 120) * 50}%`); // Execute or continue this browser control operation.
  elements.leftStick.style.setProperty("--stick-y", `${100 - throttlePercent}%`); // Execute or continue this browser control operation.
  elements.rightStick.style.setProperty("--stick-x", `${50 + (state.control.roll / 25) * 50}%`); // Execute or continue this browser control operation.
  elements.rightStick.style.setProperty("--stick-y", `${50 - (state.control.pitch / 25) * 50}%`); // Execute or continue this browser control operation.
  updateSafetyControls(); // Changing stick values can change whether the system is allowed to arm.
} // Close the current JavaScript scope or object.

function resetPilotControls(resetThrottle = true) { // Center both sticks and optionally force throttle to zero.
  if (resetThrottle) state.control.throttle = 0; // Evaluate this condition before continuing the interface update.
  state.control.roll = 0; // Update this value for the current Flight Deck state.
  state.control.pitch = 0; // Update this value for the current Flight Deck state.
  state.control.yaw = 0; // Update this value for the current Flight Deck state.
  updateControlDisplay(); // Execute or continue this browser control operation.
} // Close the current JavaScript scope or object.

function buildCommand() { // Serialize the current pilot state into one versioned command packet.
  const sequence = state.nextSequence++; // A strictly increasing sequence supports acknowledgement and latency tracking.
  const packet = [ // Disarmed commands always transmit zero throttle even if the UI retained a value.
    `PROTO=${PROTOCOL_VERSION}`, // Update this value for the current Flight Deck state.
    "TYPE=CMD", // Update this value for the current Flight Deck state.
    `SEQ=${sequence}`, // Update this value for the current Flight Deck state.
    `T=${state.armed ? state.control.throttle : 0}`, // Update this value for the current Flight Deck state.
    `R=${state.control.roll.toFixed(2)}`, // Execute or continue this browser control operation.
    `P=${state.control.pitch.toFixed(2)}`, // Execute or continue this browser control operation.
    `Y=${state.control.yaw.toFixed(2)}`, // Execute or continue this browser control operation.
    `A=${state.armRequested ? 1 : 0}`, // Update this value for the current Flight Deck state.
    `CAL=${Date.now() < state.calibrationPulseUntil ? 1 : 0}`, // Execute or continue this browser control operation.
    `STOP=${Date.now() < state.stopPulseUntil ? 1 : 0}`, // Execute or continue this browser control operation.
    `MODE=${state.flightMode}`, // Update this value for the current Flight Deck state.
    "", // Continue the current Flight Deck JavaScript declaration or operation.
  ].join(";"); // Execute or continue this browser control operation.
  return { sequence, packet }; // Return both forms so the sender can track the exact sequence it transmits.
} // Close the current JavaScript scope or object.

function sendCommand(trackLatency = true) { // Send one command when the WebSocket is open and optionally measure acknowledgement latency.
  if (!state.connected || !state.socket || state.socket.readyState !== WebSocket.OPEN) return false; // Never queue commands while disconnected or while the socket is not fully open.
  const command = buildCommand(); // Declare this interface state, element reference, or fixed configuration value.
  try { // Attempt this operation while allowing controlled failure handling.
    state.socket.send(command.packet); // Execute or continue this browser control operation.
    if (trackLatency) state.pendingCommands.set(command.sequence, performance.now()); // Keep the original send time until firmware acknowledges this sequence.
    while (state.pendingCommands.size > 30) { // Bound the map so a missing acknowledgement cannot grow memory indefinitely.
      state.pendingCommands.delete(state.pendingCommands.keys().next().value); // Execute or continue this browser control operation.
    } // Close the current JavaScript scope or object.
    return true; // Return this computed result to the caller.
  } catch (error) { // Close the current JavaScript scope or object.
    setAlert("critical", "COMMAND FAILED", error.message || "WebSocket write failed."); // A write exception is visible and does not pretend the command succeeded.
    return false; // Return this computed result to the caller.
  } // Close the current JavaScript scope or object.
} // Close the current JavaScript scope or object.

function startCommandStream() { // Start the periodic command heartbeat required by the firmware freshness failsafe.
  clearInterval(state.commandTimer); // Execute or continue this browser control operation.
  sendCommand(); // Execute or continue this browser control operation.
  state.commandTimer = setInterval(() => sendCommand(), COMMAND_PERIOD_MS); // Execute or continue this browser control operation.
} // Close the current JavaScript scope or object.

function stopCommandStream() { // Stop all periodic command transmission immediately.
  clearInterval(state.commandTimer); // Execute or continue this browser control operation.
  state.commandTimer = null; // Update this value for the current Flight Deck state.
} // Close the current JavaScript scope or object.

function disconnectFlightLink(reason = "Disconnected by operator") { // End the command link, clear arming intent, and return controls to safe values.
  stopCommandStream(); // Execute or continue this browser control operation.
  state.armRequested = false; // Update this value for the current Flight Deck state.
  state.armed = false; // Update this value for the current Flight Deck state.
  resetPilotControls(true); // Execute or continue this browser control operation.
  const socket = state.socket; // Declare this interface state, element reference, or fixed configuration value.
  state.socket = null; // Clear the active reference before close events run so stale callbacks are ignored.
  if (socket && socket.readyState < WebSocket.CLOSING) socket.close(1000, reason); // Evaluate this condition before continuing the interface update.
  setLinkState(false, reason); // Execute or continue this browser control operation.
} // Close the current JavaScript scope or object.

function connectFlightLink() { // Create and manage a direct WebSocket connection to the flight controller.
  if (state.connected || state.connecting) { // The same button disconnects an existing or pending connection.
    disconnectFlightLink(); // Execute or continue this browser control operation.
    return; // Return this computed result to the caller.
  } // Close the current JavaScript scope or object.
  if (location.protocol === "https:") { // Browsers block insecure ws:// from an HTTPS page, so explain the required local URL.
    setAlert("critical", "BROWSER SECURITY BLOCK", "Open http://192.168.4.1 on the NanoKit network. HTTPS pages cannot open the controller's plain WebSocket.");
    return; // Return this computed result to the caller.
  } // Close the current JavaScript scope or object.

  const url = websocketUrl(); // Lock the connect control while the browser creates the new socket.
  state.connecting = true; // Update this value for the current Flight Deck state.
  elements.connectButton.disabled = true; // Update this value for the current Flight Deck state.
  elements.connectButton.textContent = "Connecting"; // Update this value for the current Flight Deck state.
  setEvent(`Opening ${url}`); // Execute or continue this browser control operation.

  let socket; // Declare this interface state, element reference, or fixed configuration value.
  try { // WebSocket construction can fail immediately for an invalid endpoint.
    socket = new WebSocket(url); // Execute or continue this browser control operation.
  } catch (error) { // Close the current JavaScript scope or object.
    state.connecting = false; // Update this value for the current Flight Deck state.
    elements.connectButton.disabled = false; // Update this value for the current Flight Deck state.
    setAlert("critical", "LINK ERROR", error.message || "Unable to create WebSocket."); // Execute or continue this browser control operation.
    return; // Return this computed result to the caller.
  } // Close the current JavaScript scope or object.
  state.socket = socket; // Update this value for the current Flight Deck state.

  socket.addEventListener("open", () => { // Start the command heartbeat only after the browser confirms an open connection.
    if (state.socket !== socket) return; // Evaluate this condition before continuing the interface update.
    state.lastTelemetryAt = Date.now(); // Execute or continue this browser control operation.
    setLinkState(true, `WebSocket active at ${url}`); // Execute or continue this browser control operation.
    setAlert("warning", "SAFETY GATES ACTIVE", "The controller is connected. Arming remains blocked until IMU, calibration, and ESC protocol checks pass."); // Execute or continue this browser control operation.
    startCommandStream(); // Execute or continue this browser control operation.
  }); // Close the current JavaScript scope or object.

  socket.addEventListener("message", (event) => handleFlightPacket(event.data)); // Route protocol data and lifecycle failures to their dedicated handlers.
  socket.addEventListener("error", () => { // Open this JavaScript implementation block.
    setAlert("critical", "LINK ERROR", "The Flight Deck could not maintain the WebSocket connection."); // Execute or continue this browser control operation.
  }); // Close the current JavaScript scope or object.
  socket.addEventListener("close", (event) => { // Open this JavaScript implementation block.
    if (state.socket !== socket) return; // Evaluate this condition before continuing the interface update.
    state.socket = null; // Update this value for the current Flight Deck state.
    stopCommandStream(); // A closed flight link always removes arm intent and pilot demand locally.
    state.armRequested = false; // Update this value for the current Flight Deck state.
    state.armed = false; // Update this value for the current Flight Deck state.
    resetPilotControls(true); // Execute or continue this browser control operation.
    setLinkState(false, `WebSocket closed (${event.code})`); // Execute or continue this browser control operation.
    setAlert("critical", "FLIGHT LINK LOST", "Commands stopped. Firmware failsafe forces all motors to minimum."); // Execute or continue this browser control operation.
  }); // Close the current JavaScript scope or object.
} // Close the current JavaScript scope or object.

function updateLatency(sequence) { // Resolve one acknowledged command into a round-trip latency measurement.
  const sentAt = state.pendingCommands.get(Number(sequence)); // Declare this interface state, element reference, or fixed configuration value.
  if (sentAt === undefined) return; // Evaluate this condition before continuing the interface update.
  state.latencyMs = Math.max(0, Math.round(performance.now() - sentAt)); // performance.now() provides a monotonic high-resolution browser timer.
  state.pendingCommands.delete(Number(sequence)); // Execute or continue this browser control operation.
  elements.latencyValue.textContent = `${state.latencyMs} ms`; // Update this value for the current Flight Deck state.
} // Close the current JavaScript scope or object.

function handleFlightPacket(payload) { // Validate and apply one HELLO, ACK, ERROR, or telemetry packet from firmware.
  const fields = parsePacket(payload); // Declare this interface state, element reference, or fixed configuration value.
  if (fields.PROTO !== PROTOCOL_VERSION) { // Refuse incompatible packet layouts instead of rendering misleading values.
    setAlert("critical", "PROTOCOL MISMATCH", "Upload the current NanoKit Drone 4X Wi-Fi firmware."); // Execute or continue this browser control operation.
    return; // Return this computed result to the caller.
  } // Close the current JavaScript scope or object.

  if (fields.TYPE === "HELLO") { // HELLO confirms device identity but carries no flight telemetry.
    setEvent(`${fields.DEVICE || "NanoKit-Drone-4X"} Wi-Fi link confirmed`); // Execute or continue this browser control operation.
    return; // Return this computed result to the caller.
  } // Close the current JavaScript scope or object.
  if (fields.TYPE === "ACK") { // ACK closes the latency measurement for one sent command.
    updateLatency(fields.SEQ); // Execute or continue this browser control operation.
    return; // Return this computed result to the caller.
  } // Close the current JavaScript scope or object.
  if (fields.TYPE === "ERROR") { // Firmware validation errors remain visible to the operator.
    setAlert("critical", fields.CODE || "COMMAND ERROR", fields.MESSAGE || "Firmware rejected the command."); // Execute or continue this browser control operation.
    return; // Return this computed result to the caller.
  } // Close the current JavaScript scope or object.
  if (fields.TYPE !== "TEL") return; // Ignore unknown packet types rather than interpreting them as telemetry.

  state.lastTelemetryAt = Date.now(); // Store the complete newest telemetry packet and any included acknowledgement.
  state.telemetry = fields; // Update this value for the current Flight Deck state.
  if (fields.ACK) updateLatency(fields.ACK); // Evaluate this condition before continuing the interface update.

  const wasArmed = state.armed; // Track armed duration and clear pilot controls immediately after disarming.
  state.armed = fields.ARM === "1"; // Update this value for the current Flight Deck state.
  if (!wasArmed && state.armed) state.armedSince = Date.now(); // Evaluate this condition before continuing the interface update.
  if (wasArmed && !state.armed && state.armedSince) { // Evaluate this condition before continuing the interface update.
    state.accumulatedFlightMs += Date.now() - state.armedSince; // Execute or continue this browser control operation.
    state.armedSince = 0; // Update this value for the current Flight Deck state.
    state.armRequested = false; // Update this value for the current Flight Deck state.
    resetPilotControls(true); // Execute or continue this browser control operation.
  } // Close the current JavaScript scope or object.

  state.safety.imuHealthy = fieldEnabled(fields, "IMU"); // Copy explicit firmware validity bits before displaying any sensor number.
  state.safety.imuCalibrated = fieldEnabled(fields, "CAL"); // Execute or continue this browser control operation.
  state.safety.attitudeValid = fieldEnabled(fields, "ATT_VALID"); // Execute or continue this browser control operation.
  state.safety.escConfirmed = fieldEnabled(fields, "ESC_OK"); // Execute or continue this browser control operation.
  state.safety.gnssFix = fieldEnabled(fields, "GNSS"); // Execute or continue this browser control operation.
  state.safety.barometerValid = fieldEnabled(fields, "BARO"); // Execute or continue this browser control operation.
  state.safety.environmentValid = fieldEnabled(fields, "ENV"); // Execute or continue this browser control operation.
  state.safety.powerValid = fieldEnabled(fields, "POWER"); // Execute or continue this browser control operation.
  state.safety.navigationReady = fieldEnabled(fields, "NAV"); // Execute or continue this browser control operation.

  const attitudeValid = state.safety.imuHealthy && state.safety.attitudeValid; // Attitude values are shown only when both IMU health and fused attitude are valid.
  const roll = attitudeValid ? formatNumber(fields.ROLL) : null; // Declare this interface state, element reference, or fixed configuration value.
  const pitch = attitudeValid ? formatNumber(fields.PITCH) : null; // Declare this interface state, element reference, or fixed configuration value.
  const yaw = attitudeValid ? formatNumber(fields.YAW) : null; // Declare this interface state, element reference, or fixed configuration value.
  elements.rollValue.textContent = roll === null ? "--.- deg" : `${roll} deg`; // Update this value for the current Flight Deck state.
  elements.pitchValue.textContent = pitch === null ? "--.- deg" : `${pitch} deg`; // Update this value for the current Flight Deck state.
  elements.headingValue.textContent = yaw === null ? "--- deg" : `${yaw} deg`; // Update this value for the current Flight Deck state.

  const altitude = state.safety.barometerValid ? formatNumber(fields.ALT, 2) : null; // Barometer and GNSS values use their own independent hardware validity gates.
  elements.altitudeValue.textContent = altitude === null ? "--.- m" : `${altitude} m`; // Update this value for the current Flight Deck state.
  const speed = state.safety.gnssFix ? formatNumber(fields.SPD, 2) : null; // Declare this interface state, element reference, or fixed configuration value.
  elements.speedValue.textContent = speed === null ? "--.- m/s" : `${speed} m/s`; // Update this value for the current Flight Deck state.

  const voltage = state.safety.powerValid ? formatNumber(fields.BAT_V, 2) : null; // Power telemetry is hidden as unavailable until a real power monitor is valid.
  const current = state.safety.powerValid ? formatNumber(fields.BAT_A, 2) : null; // Declare this interface state, element reference, or fixed configuration value.
  const power = state.safety.powerValid ? formatNumber(fields.BAT_W, 1) : null; // Declare this interface state, element reference, or fixed configuration value.
  elements.voltageValue.textContent = voltage === null ? "--.- V" : `${voltage} V`; // Update this value for the current Flight Deck state.
  elements.currentValue.textContent = current === null ? "--.- A" : `${current} A`; // Update this value for the current Flight Deck state.
  elements.powerValue.textContent = power === null ? "--.- W" : `${power} W`; // Update this value for the current Flight Deck state.
  elements.batteryTop.textContent = voltage === null ? "--.- V" : `${voltage} V`; // Update this value for the current Flight Deck state.

  elements.gnssTop.textContent = state.safety.gnssFix ? "FIX" : "NO FIX"; // GNSS position, command age, mode, optical flow, and obstacles update independently.
  elements.satelliteValue.textContent = state.safety.gnssFix ? (fields.SAT || "0") : "--"; // Execute or continue this browser control operation.
  elements.coordinateValue.textContent = state.safety.gnssFix // Update this value for the current Flight Deck state.
    ? `${formatNumber(fields.LAT, 7)}, ${formatNumber(fields.LON, 7)}` // Execute or continue this browser control operation.
    : "--.-------, --.-------"; // Execute this JavaScript statement for the interface.
  elements.commandAgeValue.textContent = fields.CMD_AGE ? `${fields.CMD_AGE} ms` : "-- ms"; // Update this value for the current Flight Deck state.
  elements.modeValue.textContent = fields.MODE || state.flightMode; // Update this value for the current Flight Deck state.
  elements.flowValue.textContent = fieldEnabled(fields, "FLOW") ? "TRACKING" : "OFFLINE"; // Execute or continue this browser control operation.
  elements.obstacleValue.textContent = fieldEnabled(fields, "OBST") ? "ACTIVE" : "OFFLINE"; // Execute or continue this browser control operation.

  const temperature = state.safety.environmentValid ? formatNumber(fields.TEMP) : null; // Environmental values remain placeholders unless the firmware marks them valid.
  const humidity = state.safety.environmentValid ? formatNumber(fields.HUM) : null; // Declare this interface state, element reference, or fixed configuration value.
  elements.temperatureValue.textContent = temperature === null ? "--.- degC" : `${temperature} degC`; // Update this value for the current Flight Deck state.
  elements.humidityValue.textContent = humidity === null ? "--%" : `${humidity}%`; // Update this value for the current Flight Deck state.

  const telemetryThrottle = state.armed ? Number(fields.THR || 0) : 0; // Mirror telemetry throttle only while the firmware confirms an armed state.
  if (state.armed && Number.isFinite(telemetryThrottle)) state.control.throttle = clamp(telemetryThrottle, 0, 1000); // Evaluate this condition before continuing the interface update.
  updateControlDisplay(); // Execute or continue this browser control operation.
  updateSafetyControls(); // Execute or continue this browser control operation.
  presentSafetyStatus(fields); // Present the firmware status after all supporting state has been updated.
} // Close the current JavaScript scope or object.

function presentSafetyStatus(fields) { // Translate authoritative firmware safety fields into one operator alert.
  const status = fields.STATUS || "Safety state updated"; // Declare this interface state, element reference, or fixed configuration value.
  if (fields.FAILSAFE === "1" || fields.STATE === "FAILSAFE" || fields.STATE === "FAULT") { // Fault and failsafe states always override lower-priority readiness messages.
    setAlert("critical", fields.STATE || "FAULT", status); // Execute or continue this browser control operation.
  } else if (state.armed) { // Close the current JavaScript scope or object.
    setAlert("good", "AIRCRAFT ARMED", status); // Execute or continue this browser control operation.
  } else if (!state.safety.imuHealthy) { // Close the current JavaScript scope or object.
    setAlert("critical", "IMU UNAVAILABLE", status); // Execute or continue this browser control operation.
  } else if (!state.safety.imuCalibrated) { // Close the current JavaScript scope or object.
    setAlert("warning", "CALIBRATION REQUIRED", status); // Execute or continue this browser control operation.
  } else if (!state.safety.escConfirmed) { // Close the current JavaScript scope or object.
    setAlert("warning", "ESC OUTPUT LOCKED", status); // Execute or continue this browser control operation.
  } else { // Close the current JavaScript scope or object.
    setAlert("good", "READY TO ARM", status); // Execute or continue this browser control operation.
  } // Close the current JavaScript scope or object.
  setEvent(status); // Execute or continue this browser control operation.
} // Close the current JavaScript scope or object.

function beginArmHold(event) { // Begin the intentional hold-to-arm timer without sending an immediate arm request.
  event.preventDefault(); // Execute or continue this browser control operation.
  if (elements.armHoldButton.disabled || state.armHoldTimer) return; // Evaluate this condition before continuing the interface update.
  elements.armHoldButton.classList.add("holding"); // Execute or continue this browser control operation.
  state.armHoldTimer = setTimeout(() => { // The delayed callback sends Arm only after the full safety hold duration.
    state.armHoldTimer = null; // Update this value for the current Flight Deck state.
    elements.armHoldButton.classList.remove("holding"); // Execute or continue this browser control operation.
    state.armRequested = true; // Update this value for the current Flight Deck state.
    sendCommand(false); // Execute or continue this browser control operation.
    setEvent("ARM requested; waiting for firmware safety approval"); // Execute or continue this browser control operation.
  }, ARM_HOLD_MS); // Close the current JavaScript scope or object.
} // Close the current JavaScript scope or object.

function cancelArmHold() { // Cancel a partial hold so a short click or pointer exit cannot arm the aircraft.
  clearTimeout(state.armHoldTimer); // Execute or continue this browser control operation.
  state.armHoldTimer = null; // Update this value for the current Flight Deck state.
  elements.armHoldButton.classList.remove("holding"); // Execute or continue this browser control operation.
} // Close the current JavaScript scope or object.

function disarm() { // Clear arm intent, zero controls, and send a normal disarm command.
  cancelArmHold(); // Execute or continue this browser control operation.
  state.armRequested = false; // Update this value for the current Flight Deck state.
  resetPilotControls(true); // Execute or continue this browser control operation.
  sendCommand(false); // Execute or continue this browser control operation.
  setEvent("DISARM command sent"); // Execute or continue this browser control operation.
} // Close the current JavaScript scope or object.

function emergencyStop() { // Send the one-shot emergency flag and force all local pilot values to zero.
  cancelArmHold(); // Execute or continue this browser control operation.
  state.armRequested = false; // Update this value for the current Flight Deck state.
  state.stopPulseUntil = Date.now() + 350; // Keep STOP high for several periodic packets so brief network timing cannot miss it.
  resetPilotControls(true); // Execute or continue this browser control operation.
  sendCommand(false); // Execute or continue this browser control operation.
  setAlert("critical", "EMERGENCY STOP", "Emergency stop sent. Firmware latches the arm inhibit and forces every motor to minimum."); // Execute or continue this browser control operation.
} // Close the current JavaScript scope or object.

function requestCalibration() { // Send a short calibration flag only when firmware-reported gates permit it.
  if (elements.calibrateImuButton.disabled) return; // Evaluate this condition before continuing the interface update.
  state.calibrationPulseUntil = Date.now() + 350; // Execute or continue this browser control operation.
  sendCommand(false); // Execute or continue this browser control operation.
  setEvent("IMU calibration requested; keep the frame level and motionless"); // Execute or continue this browser control operation.
} // Close the current JavaScript scope or object.

function bindStick(zone, side) { // Bind one pointer-controlled virtual stick to the appropriate pilot axes.
  let pointerId = null; // Track one active pointer so unrelated touch or mouse events are ignored.

  const updateFromPointer = (event) => { // Convert pointer position into normalized throttle/yaw or roll/pitch commands.
    if (!state.connected || event.pointerId !== pointerId) return; // Evaluate this condition before continuing the interface update.
    const rect = zone.getBoundingClientRect(); // Declare this interface state, element reference, or fixed configuration value.
    const x = clamp((event.clientX - rect.left) / rect.width, 0, 1); // Declare this interface state, element reference, or fixed configuration value.
    const y = clamp((event.clientY - rect.top) / rect.height, 0, 1); // Declare this interface state, element reference, or fixed configuration value.
    if (side === "left") { // The left stick controls yaw and armed throttle; the right controls attitude.
      state.control.yaw = (x * 2 - 1) * 120; // Execute or continue this browser control operation.
      if (state.armed) state.control.throttle = Math.round((1 - y) * 1000); // Evaluate this condition before continuing the interface update.
    } else { // Close the current JavaScript scope or object.
      state.control.roll = (x * 2 - 1) * 25; // Execute or continue this browser control operation.
      state.control.pitch = (1 - y * 2) * 25; // Execute or continue this browser control operation.
    } // Close the current JavaScript scope or object.
    updateControlDisplay(); // Execute or continue this browser control operation.
  }; // Close the current JavaScript scope or object.

  zone.addEventListener("pointerdown", (event) => { // Capture the pointer so control remains stable if it moves outside the stick element.
    if (!state.connected) { // Evaluate this condition before continuing the interface update.
      setAlert("warning", "NO FLIGHT LINK", "Connect the WebSocket command link before using the sticks."); // Execute or continue this browser control operation.
      return; // Return this computed result to the caller.
    } // Close the current JavaScript scope or object.
    pointerId = event.pointerId; // Update this value for the current Flight Deck state.
    zone.setPointerCapture(pointerId); // Execute or continue this browser control operation.
    zone.classList.add("dragging"); // Execute or continue this browser control operation.
    updateFromPointer(event); // Execute or continue this browser control operation.
  }); // Close the current JavaScript scope or object.
  zone.addEventListener("pointermove", updateFromPointer); // Execute or continue this browser control operation.
  const release = (event) => { // Releasing either stick centers its attitude axes; throttle remains where commanded.
    if (event.pointerId !== pointerId) return; // Evaluate this condition before continuing the interface update.
    zone.classList.remove("dragging"); // Execute or continue this browser control operation.
    pointerId = null; // Update this value for the current Flight Deck state.
    if (side === "left") state.control.yaw = 0; // Evaluate this condition before continuing the interface update.
    else { // Handle the alternative result of the preceding condition.
      state.control.roll = 0; // Update this value for the current Flight Deck state.
      state.control.pitch = 0; // Update this value for the current Flight Deck state.
    } // Close the current JavaScript scope or object.
    updateControlDisplay(); // Execute or continue this browser control operation.
  }; // Close the current JavaScript scope or object.
  zone.addEventListener("pointerup", release); // Execute or continue this browser control operation.
  zone.addEventListener("pointercancel", release); // Execute or continue this browser control operation.
} // Close the current JavaScript scope or object.

function switchView(view) { // Switch between standard, FPV, camera, and local mission interface views.
  state.view = view; // Update this value for the current Flight Deck state.
  document.querySelectorAll(".mode-button").forEach((button) => { // Open this JavaScript implementation block.
    button.classList.toggle("active", button.dataset.view === view); // Execute or continue this browser control operation.
  }); // Close the current JavaScript scope or object.
  elements.flightDeck.classList.toggle("fpv-mode", view === "fpv"); // CSS classes control presentation while hidden flags select stage or mission content.
  elements.flightDeck.classList.toggle("camera-mode", view === "camera"); // Execute or continue this browser control operation.
  elements.videoStage.hidden = view === "mission"; // Update this value for the current Flight Deck state.
  elements.missionWorkspace.hidden = view !== "mission"; // Update this value for the current Flight Deck state.
} // Close the current JavaScript scope or object.

function chooseFlightMode(mode) { // Select a requested flight mode and let firmware perform final mode validation.
  state.flightMode = mode; // Update this value for the current Flight Deck state.
  document.querySelectorAll(".flight-mode").forEach((button) => { // Open this JavaScript implementation block.
    button.classList.toggle("active", button.dataset.flightMode === mode); // Execute or continue this browser control operation.
  }); // Close the current JavaScript scope or object.
  elements.modeValue.textContent = mode; // Update this value for the current Flight Deck state.
  sendCommand(false); // Send immediately so the operator does not wait for the next heartbeat.
} // Close the current JavaScript scope or object.

function setCameraLive(live, message) { // Toggle camera-stage presentation while preserving an explicit status message.
  state.cameraLive = live; // Update this value for the current Flight Deck state.
  elements.cameraFeed.hidden = !live; // Update this value for the current Flight Deck state.
  elements.offlineReference.hidden = live; // Update this value for the current Flight Deck state.
  setStatusDot(elements.cameraSignalDot, live ? "good" : "warning"); // Execute or continue this browser control operation.
  elements.cameraSignalText.textContent = live ? "CAMERA LIVE" : "CAMERA STANDBY"; // Update this value for the current Flight Deck state.
  elements.streamMode.textContent = live ? "LIVE CAMERA" : "REFERENCE VIEW"; // Update this value for the current Flight Deck state.
  elements.frameInfo.textContent = message; // Update this value for the current Flight Deck state.
} // Close the current JavaScript scope or object.

function loadCameraStream() { // Load the configured observation stream with a cache-busting query value.
  if (!state.cameraUrl) { // Evaluate this condition before continuing the interface update.
    setCameraLive(false, "Camera stream URL is not configured"); // Execute or continue this browser control operation.
    return; // Return this computed result to the caller.
  } // Close the current JavaScript scope or object.
  elements.cameraEndpointLabel.textContent = state.cameraUrl; // Update this value for the current Flight Deck state.
  elements.cameraFeed.onload = () => setCameraLive(true, "Observation stream online"); // Browser image load events are the source of truth for visible stream status.
  elements.cameraFeed.onerror = () => setCameraLive(false, "Camera endpoint unavailable"); // Execute or continue this browser control operation.
  elements.cameraFeed.src = `${state.cameraUrl}${state.cameraUrl.includes("?") ? "&" : "?"}t=${Date.now()}`; // Execute or continue this browser control operation.
} // Close the current JavaScript scope or object.

async function refreshPayloadStatus() { // Poll the separate camera node and update its independent hardware gates.
  if (!state.cameraApiUrl) return; // Evaluate this condition before continuing the interface update.
  const controller = new AbortController(); // Abort a slow payload request so it cannot accumulate behind later polls.
  const timeout = setTimeout(() => controller.abort(), 2500); // Declare this interface state, element reference, or fixed configuration value.
  try { // Attempt this operation while allowing controlled failure handling.
    const response = await fetch(`${state.cameraApiUrl.replace(/\/$/, "")}/api/status`, { // Disable browser caching because readiness can change after a hardware reboot.
      signal: controller.signal, // Continue the current Flight Deck JavaScript declaration or operation.
      cache: "no-store", // Continue the current Flight Deck JavaScript declaration or operation.
    }); // Close the current JavaScript scope or object.
    if (!response.ok) throw new Error(`HTTP ${response.status}`); // Evaluate this condition before continuing the interface update.
    const payload = await response.json(); // Accept only strict JSON booleans to avoid treating text as hardware readiness.
    state.payload.cameraReady = payload.cameraReady === true; // Update this value for the current Flight Deck state.
    state.payload.audioCaptureReady = payload.audioCaptureReady === true; // Update this value for the current Flight Deck state.
    state.payload.audioPlaybackReady = payload.audioPlaybackReady === true; // Update this value for the current Flight Deck state.
    state.payload.storageReady = payload.storageReady === true; // Update this value for the current Flight Deck state.
    state.payload.psram8MB = payload.psram8MB === true; // Update this value for the current Flight Deck state.
    elements.payloadState.textContent = state.payload.cameraReady ? "READY" : "LOCKED"; // Update this value for the current Flight Deck state.
  } catch (_error) { // Close the current JavaScript scope or object.
    Object.keys(state.payload).forEach((key) => { state.payload[key] = false; }); // Any timeout, network, or parse failure returns every payload control to offline.
    elements.payloadState.textContent = "OFFLINE"; // Update this value for the current Flight Deck state.
  } finally { // Close the current JavaScript scope or object.
    clearTimeout(timeout); // Always clear the timer and re-evaluate payload controls after the request finishes.
    updatePayloadControls(); // Execute or continue this browser control operation.
  } // Close the current JavaScript scope or object.
} // Close the current JavaScript scope or object.

function updatePayloadControls() { // Enable each payload button only when its exact hardware dependency is ready.
  elements.snapshotButton.disabled = !state.payload.cameraReady; // Update this value for the current Flight Deck state.
  elements.recordButton.disabled = !(state.payload.cameraReady && state.payload.storageReady); // Execute or continue this browser control operation.
  elements.audioButton.disabled = !state.payload.audioCaptureReady; // Update this value for the current Flight Deck state.
  elements.speakerButton.disabled = !state.payload.audioPlaybackReady; // Update this value for the current Flight Deck state.
  elements.cameraTilt.disabled = !state.payload.cameraReady; // Update this value for the current Flight Deck state.
} // Close the current JavaScript scope or object.

async function postPayload(path, body) { // Send one JSON command to the camera node and surface any rejection.
  try { // Attempt this operation while allowing controlled failure handling.
    const response = await fetch(`${state.cameraApiUrl.replace(/\/$/, "")}${path}`, { // Payload endpoints are separate HTTP requests and cannot control flight motors.
      method: "POST", // Continue the current Flight Deck JavaScript declaration or operation.
      headers: { "Content-Type": "text/plain" }, // Continue the current Flight Deck JavaScript declaration or operation.
      body: JSON.stringify(body), // Execute or continue this browser control operation.
    }); // Close the current JavaScript scope or object.
    const payload = await response.json().catch(() => ({})); // Declare this interface state, element reference, or fixed configuration value.
    if (!response.ok) throw new Error(payload.reason || `HTTP ${response.status}`); // Non-success responses use the node's reason when one is available.
    return payload; // Return this computed result to the caller.
  } catch (error) { // Close the current JavaScript scope or object.
    setAlert("warning", "PAYLOAD COMMAND REJECTED", error.message || "Camera node unavailable."); // Execute or continue this browser control operation.
    return null; // Return this computed result to the caller.
  } // Close the current JavaScript scope or object.
} // Close the current JavaScript scope or object.

function readStoredMission() { // Restore up to fifty local planning waypoints from browser storage.
  try { // Attempt this operation while allowing controlled failure handling.
    const stored = JSON.parse(localStorage.getItem(MISSION_KEY) || "[]"); // Declare this interface state, element reference, or fixed configuration value.
    if (Array.isArray(stored)) state.waypoints = stored.slice(0, 50); // The limit bounds rendering work and the size of locally saved mission data.
  } catch (_error) { // Close the current JavaScript scope or object.
    state.waypoints = []; // Update this value for the current Flight Deck state.
  } // Close the current JavaScript scope or object.
} // Close the current JavaScript scope or object.

function renderMission() { // Render the local mission path, numbered markers, and readable waypoint list.
  elements.missionPath.replaceChildren(); // Rebuild the SVG from state so deleted or reordered points cannot leave stale nodes.
  const namespace = "http://www.w3.org/2000/svg";
  if (state.waypoints.length > 1) { // Evaluate this condition before continuing the interface update.
    const path = document.createElementNS(namespace, "polyline"); // Draw one continuous polyline through all staged points.
    path.setAttribute("points", state.waypoints.map((point) => `${point.x * 10},${point.y * 6}`).join(" ")); // Execute or continue this browser control operation.
    path.setAttribute("fill", "none"); // Execute or continue this browser control operation.
    path.setAttribute("stroke", "#72d6ef"); // Execute or continue this browser control operation.
    path.setAttribute("stroke-width", "3"); // Execute or continue this browser control operation.
    path.setAttribute("vector-effect", "non-scaling-stroke"); // Execute or continue this browser control operation.
    elements.missionPath.append(path); // Execute or continue this browser control operation.
  } // Close the current JavaScript scope or object.
  state.waypoints.forEach((point, index) => { // Add one numbered SVG circle for every waypoint.
    const marker = document.createElementNS(namespace, "circle"); // Declare this interface state, element reference, or fixed configuration value.
    marker.setAttribute("cx", String(point.x * 10)); // Execute or continue this browser control operation.
    marker.setAttribute("cy", String(point.y * 6)); // Execute or continue this browser control operation.
    marker.setAttribute("r", "8"); // Execute or continue this browser control operation.
    marker.setAttribute("fill", "#070b0e"); // Execute or continue this browser control operation.
    marker.setAttribute("stroke", "#a8df62"); // Execute or continue this browser control operation.
    marker.setAttribute("stroke-width", "3"); // Execute or continue this browser control operation.
    marker.setAttribute("vector-effect", "non-scaling-stroke"); // Execute or continue this browser control operation.
    elements.missionPath.append(marker); // Execute or continue this browser control operation.
    const label = document.createElementNS(namespace, "text"); // Declare this interface state, element reference, or fixed configuration value.
    label.setAttribute("x", String(point.x * 10)); // Execute or continue this browser control operation.
    label.setAttribute("y", String(point.y * 6 + 3)); // Execute or continue this browser control operation.
    label.setAttribute("text-anchor", "middle"); // Execute or continue this browser control operation.
    label.setAttribute("fill", "#edf5f7"); // Execute or continue this browser control operation.
    label.setAttribute("font-size", "9"); // Execute or continue this browser control operation.
    label.textContent = String(index + 1); // Execute or continue this browser control operation.
    elements.missionPath.append(label); // Execute or continue this browser control operation.
  }); // Close the current JavaScript scope or object.

  elements.waypointList.replaceChildren(); // Rebuild the text list to match the same source waypoint array.
  if (state.waypoints.length === 0) { // Evaluate this condition before continuing the interface update.
    const empty = document.createElement("li"); // Declare this interface state, element reference, or fixed configuration value.
    empty.className = "empty-waypoints"; // Update this value for the current Flight Deck state.
    empty.textContent = "No waypoints staged"; // Update this value for the current Flight Deck state.
    elements.waypointList.append(empty); // Execute or continue this browser control operation.
  } else { // Close the current JavaScript scope or object.
    state.waypoints.forEach((point, index) => { // Open this JavaScript implementation block.
      const item = document.createElement("li"); // Declare this interface state, element reference, or fixed configuration value.
      item.textContent = `WP${index + 1}  X ${point.x.toFixed(1)}  Y ${point.y.toFixed(1)}`; // Execute or continue this browser control operation.
      elements.waypointList.append(item); // Execute or continue this browser control operation.
    }); // Close the current JavaScript scope or object.
  } // Close the current JavaScript scope or object.
  localStorage.setItem(MISSION_KEY, JSON.stringify(state.waypoints)); // Persist local planning only; this does not upload a mission to the aircraft.
  updateSafetyControls(); // Execute or continue this browser control operation.
} // Close the current JavaScript scope or object.

function saveMissionFile() { // Download the staged local mission as a clearly marked non-uploaded JSON file.
  const documentBody = { // Declare this interface state, element reference, or fixed configuration value.
    format: "nanokit-drone-4x-local-mission", // Continue the current Flight Deck JavaScript declaration or operation.
    version: 1, // Continue the current Flight Deck JavaScript declaration or operation.
    uploaded: false, // Continue the current Flight Deck JavaScript declaration or operation.
    note: "Local planning only until GNSS and navigation feature gates are verified.", // Continue the current Flight Deck JavaScript declaration or operation.
    waypoints: state.waypoints, // Continue the current Flight Deck JavaScript declaration or operation.
  }; // Close the current JavaScript scope or object.
  const blob = new Blob([JSON.stringify(documentBody, null, 2)], { type: "application/json" }); // A temporary object URL lets the browser download generated JSON without a server.
  const link = document.createElement("a"); // Declare this interface state, element reference, or fixed configuration value.
  link.href = URL.createObjectURL(blob); // Execute or continue this browser control operation.
  link.download = "nanokit-drone-4x-mission.json"; // Update this value for the current Flight Deck state.
  link.click(); // Execute or continue this browser control operation.
  URL.revokeObjectURL(link.href); // Release the temporary URL immediately after starting the download.
} // Close the current JavaScript scope or object.

function updateClocks() { // Update armed flight time and warn when a connected link stops receiving telemetry.
  const liveFlightMs = state.armed && state.armedSince ? Date.now() - state.armedSince : 0; // Declare this interface state, element reference, or fixed configuration value.
  const totalSeconds = Math.floor((state.accumulatedFlightMs + liveFlightMs) / 1000); // Declare this interface state, element reference, or fixed configuration value.
  const minutes = String(Math.floor(totalSeconds / 60)).padStart(2, "0"); // Declare this interface state, element reference, or fixed configuration value.
  const seconds = String(totalSeconds % 60).padStart(2, "0"); // Declare this interface state, element reference, or fixed configuration value.
  elements.flightTimeValue.textContent = `${minutes}:${seconds}`; // Update this value for the current Flight Deck state.
  if (state.connected && Date.now() - state.lastTelemetryAt > 1200) { // A stale display warns the operator before the firmware command timeout is forgotten.
    setStatusDot(elements.linkDot, "warning"); // Execute or continue this browser control operation.
    elements.linkText.textContent = "STALE"; // Update this value for the current Flight Deck state.
  } // Close the current JavaScript scope or object.
} // Close the current JavaScript scope or object.

function openSettings() { // Populate and open the endpoint settings dialog.
  elements.flightHost.value = state.flightHost; // Update this value for the current Flight Deck state.
  elements.cameraUrl.value = state.cameraUrl; // Update this value for the current Flight Deck state.
  elements.cameraApiUrl.value = state.cameraApiUrl; // Update this value for the current Flight Deck state.
  elements.settingsDialog.showModal(); // Execute or continue this browser control operation.
} // Close the current JavaScript scope or object.

function saveSettings() { // Validate, persist, and apply operator endpoint settings.
  const previousHost = normalizeHost(state.flightHost); // Remember the previous host so an active link can be closed when the endpoint changes.
  state.flightHost = normalizeHost(elements.flightHost.value); // Execute or continue this browser control operation.
  state.cameraUrl = elements.cameraUrl.value.trim(); // Execute or continue this browser control operation.
  state.cameraApiUrl = elements.cameraApiUrl.value.trim().replace(/\/$/, ""); // Execute or continue this browser control operation.
  localStorage.setItem(SETTINGS_KEY, JSON.stringify({ // Open this JavaScript implementation block.
    flightHost: state.flightHost, // Continue the current Flight Deck JavaScript declaration or operation.
    cameraUrl: state.cameraUrl, // Continue the current Flight Deck JavaScript declaration or operation.
    cameraApiUrl: state.cameraApiUrl, // Continue the current Flight Deck JavaScript declaration or operation.
  })); // Close the current JavaScript scope or object.
  elements.cameraEndpointLabel.textContent = state.cameraUrl || "No stream endpoint"; // Refresh both the flight link decision and separate payload endpoints.
  if (state.connected && previousHost !== state.flightHost) disconnectFlightLink("Flight host changed"); // Evaluate this condition before continuing the interface update.
  loadCameraStream(); // Execute or continue this browser control operation.
  refreshPayloadStatus(); // Execute or continue this browser control operation.
} // Close the current JavaScript scope or object.

elements.connectButton.addEventListener("click", connectFlightLink); // Connect top-level flight, settings, alert, arming, and emergency controls.
elements.settingsButton.addEventListener("click", openSettings); // Execute or continue this browser control operation.
elements.dismissAlert.addEventListener("click", () => { elements.alertBar.hidden = true; }); // Execute or continue this browser control operation.
elements.armHoldButton.addEventListener("pointerdown", beginArmHold); // Execute or continue this browser control operation.
elements.armHoldButton.addEventListener("pointerup", cancelArmHold); // Execute or continue this browser control operation.
elements.armHoldButton.addEventListener("pointerleave", cancelArmHold); // Execute or continue this browser control operation.
elements.armHoldButton.addEventListener("pointercancel", cancelArmHold); // Execute or continue this browser control operation.
elements.disarmButton.addEventListener("click", disarm); // Execute or continue this browser control operation.
elements.calibrateImuButton.addEventListener("click", requestCalibration); // Execute or continue this browser control operation.
elements.emergencyButton.addEventListener("click", emergencyStop); // Execute or continue this browser control operation.
elements.fullscreenButton.addEventListener("click", () => { // Fullscreen is limited to the video stage and reports browser rejection in the event log.
  if (document.fullscreenElement) document.exitFullscreen(); // Evaluate this condition before continuing the interface update.
  else elements.videoStage.requestFullscreen().catch((error) => setEvent(error.message)); // Handle the alternative result of the preceding condition.
}); // Close the current JavaScript scope or object.

document.querySelectorAll(".mode-button").forEach((button) => { // View buttons change presentation only; flight-mode buttons send a validated request.
  button.addEventListener("click", () => switchView(button.dataset.view)); // Execute or continue this browser control operation.
}); // Close the current JavaScript scope or object.
document.querySelectorAll(".flight-mode").forEach((button) => { // Open this JavaScript implementation block.
  button.addEventListener("click", () => chooseFlightMode(button.dataset.flightMode)); // Execute or continue this browser control operation.
}); // Close the current JavaScript scope or object.

bindStick(elements.leftStick, "left"); // Bind both virtual sticks after their elements and shared state are available.
bindStick(elements.rightStick, "right"); // Execute or continue this browser control operation.

elements.snapshotButton.addEventListener("click", () => postPayload("/api/camera", { action: "snapshot" })); // Payload controls call only the separate camera-node API.
elements.recordButton.addEventListener("click", async () => { // Open this JavaScript implementation block.
  const next = !state.recording; // Update local recording presentation only after the node accepts the request.
  if (await postPayload("/api/recording", { action: next ? "start" : "stop" })) { // Evaluate this condition before continuing the interface update.
    state.recording = next; // Update this value for the current Flight Deck state.
    elements.recordButton.classList.toggle("active", next); // Execute or continue this browser control operation.
    elements.recordButton.lastChild.textContent = next ? " Stop" : " Record"; // Update this value for the current Flight Deck state.
  } // Close the current JavaScript scope or object.
}); // Close the current JavaScript scope or object.
elements.audioButton.addEventListener("click", async () => { // Open this JavaScript implementation block.
  const next = !state.audioMonitoring; // Microphone monitoring and speaker output use independent readiness gates.
  if (await postPayload("/api/audio", { action: next ? "monitor-on" : "monitor-off" })) { // Evaluate this condition before continuing the interface update.
    state.audioMonitoring = next; // Update this value for the current Flight Deck state.
    elements.audioButton.classList.toggle("active", next); // Execute or continue this browser control operation.
  } // Close the current JavaScript scope or object.
}); // Close the current JavaScript scope or object.
elements.speakerButton.addEventListener("click", async () => { // Open this JavaScript implementation block.
  const next = !state.speakerActive; // Declare this interface state, element reference, or fixed configuration value.
  if (await postPayload("/api/audio", { action: next ? "speaker-on" : "speaker-off" })) { // Evaluate this condition before continuing the interface update.
    state.speakerActive = next; // Update this value for the current Flight Deck state.
    elements.speakerButton.classList.toggle("active", next); // Execute or continue this browser control operation.
  } // Close the current JavaScript scope or object.
}); // Close the current JavaScript scope or object.
elements.cameraTilt.addEventListener("input", () => { // Open this JavaScript implementation block.
  elements.cameraTiltValue.textContent = `${elements.cameraTilt.value} deg`; // Input updates the preview label while change sends the final servo request.
}); // Close the current JavaScript scope or object.
elements.cameraTilt.addEventListener("change", () => { // Open this JavaScript implementation block.
  postPayload("/api/servo", { angle: Number(elements.cameraTilt.value) }); // Execute or continue this browser control operation.
}); // Close the current JavaScript scope or object.

elements.missionMap.addEventListener("click", (event) => { // Clicking the local map stages a bounded waypoint but never uploads it automatically.
  if (event.target.closest("p") || state.waypoints.length >= 50) return; // Evaluate this condition before continuing the interface update.
  const rect = elements.missionMap.getBoundingClientRect(); // Store percentage coordinates so the plan remains responsive across screen sizes.
  state.waypoints.push({ // Open this JavaScript implementation block.
    x: clamp(((event.clientX - rect.left) / rect.width) * 100, 0, 100), // Execute or continue this browser control operation.
    y: clamp(((event.clientY - rect.top) / rect.height) * 100, 0, 100), // Execute or continue this browser control operation.
  }); // Close the current JavaScript scope or object.
  renderMission(); // Execute or continue this browser control operation.
}); // Close the current JavaScript scope or object.
elements.clearMissionButton.addEventListener("click", () => { // Clear and save actions affect local mission data only.
  state.waypoints = []; // Update this value for the current Flight Deck state.
  renderMission(); // Execute or continue this browser control operation.
}); // Close the current JavaScript scope or object.
elements.saveMissionButton.addEventListener("click", saveMissionFile); // Execute or continue this browser control operation.
elements.uploadMissionButton.addEventListener("click", () => { // Open this JavaScript implementation block.
  setAlert("warning", "NAVIGATION LOCKED", "Mission upload stays disabled until GNSS and navigation hardware are verified."); // Keep upload visibly locked until GNSS and navigation feature gates are verified.
}); // Close the current JavaScript scope or object.

elements.settingsForm.addEventListener("submit", () => saveSettings()); // The settings form applies values while close and cancel only dismiss the dialog.
elements.closeSettings.addEventListener("click", () => elements.settingsDialog.close()); // Execute or continue this browser control operation.
elements.cancelSettings.addEventListener("click", () => elements.settingsDialog.close()); // Execute or continue this browser control operation.

document.addEventListener("keydown", (event) => { // Escape is an emergency shortcut only when it is not being used to close settings.
  if (event.key === "Escape" && !elements.settingsDialog.open) emergencyStop(); // Evaluate this condition before continuing the interface update.
}); // Close the current JavaScript scope or object.
document.addEventListener("visibilitychange", () => { // Hiding the page while connected requests disarming as an additional operator safeguard.
  if (document.hidden && state.connected) disarm(); // Evaluate this condition before continuing the interface update.
}); // Close the current JavaScript scope or object.
window.addEventListener("beforeunload", () => { // A final best-effort zero-throttle packet is sent before the browser unloads.
  state.armRequested = false; // Update this value for the current Flight Deck state.
  state.control.throttle = 0; // Update this value for the current Flight Deck state.
  sendCommand(false); // Execute or continue this browser control operation.
}); // Close the current JavaScript scope or object.

readStoredMission(); // Initialize local mission, display, camera, periodic status polling, and the locked state.
renderMission(); // Execute or continue this browser control operation.
elements.cameraEndpointLabel.textContent = state.cameraUrl || "No stream endpoint"; // Update this value for the current Flight Deck state.
updateControlDisplay(); // Execute or continue this browser control operation.
updatePayloadControls(); // Execute or continue this browser control operation.
switchView("standard"); // Execute or continue this browser control operation.
loadCameraStream(); // Execute or continue this browser control operation.
refreshPayloadStatus(); // Execute or continue this browser control operation.
state.uiTimer = setInterval(updateClocks, 250); // Execute or continue this browser control operation.
state.payloadTimer = setInterval(refreshPayloadStatus, 5000); // Payload readiness changes slowly, so a five-second poll avoids unnecessary requests.
setLinkState(false, "Flight Deck ready; Wi-Fi link not connected"); // Execute or continue this browser control operation.
setAlert("warning", "SYSTEM LOCKED", "Connect to NanoKit-Drone-4X Wi-Fi, open http://192.168.4.1, then connect the WebSocket link.");
