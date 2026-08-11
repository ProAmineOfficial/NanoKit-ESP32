// NanoKit Drone 4X Wi-Fi Flight Deck.
// Developed by Amine Saoud ibn al-Bashir.
// Only validated telemetry is rendered. Missing sensors remain visibly offline.

"use strict";

// Keep browser packets synchronized with firmware protocol version 3.
const PROTOCOL_VERSION = "3";
// Send commands often enough to remain inside the firmware's 600 ms timeout.
const COMMAND_PERIOD_MS = 100;
// Require an intentional hold before requesting motor authority.
const ARM_HOLD_MS = 1500;
// Local development pages still connect to the NanoKit SoftAP host by default.
const DEFAULT_FLIGHT_HOST = ["127.0.0.1", "localhost"].includes(location.hostname)
  ? "192.168.4.1"
  : location.hostname || "192.168.4.1";
// Camera defaults use the separate payload node's mDNS hostname.
const DEFAULT_CAMERA_URL = "http://nanokit-camera.local/stream";
const DEFAULT_CAMERA_API_URL = "http://nanokit-camera.local";
// Versioned keys prevent older local settings from corrupting this interface version.
const SETTINGS_KEY = "nanokit-drone-4x-flight-deck-settings-v3";
const MISSION_KEY = "nanokit-drone-4x-local-mission";

// Clamp untrusted pointer and telemetry numbers to their valid interface ranges.
const clamp = (value, minimum, maximum) => Math.max(minimum, Math.min(maximum, value));
// Centralize DOM lookup so the element map below stays concise and readable.
const byId = (id) => document.getElementById(id);

// Cache every interface element once instead of querying the DOM during control updates.
const elements = {
  // Top-level link, safety, status, and settings elements.
  flightDeck: byId("flightDeck"),
  linkDot: byId("linkDot"),
  linkText: byId("linkText"),
  gnssTop: byId("gnssTop"),
  armTop: byId("armTop"),
  batteryTop: byId("batteryTop"),
  connectButton: byId("connectButton"),
  settingsButton: byId("settingsButton"),
  alertBar: byId("alertBar"),
  alertTitle: byId("alertTitle"),
  alertMessage: byId("alertMessage"),
  dismissAlert: byId("dismissAlert"),
  // Camera stage, HUD, live telemetry, and endpoint elements.
  videoStage: byId("videoStage"),
  offlineReference: byId("offlineReference"),
  cameraFeed: byId("cameraFeed"),
  cameraSignalDot: byId("cameraSignalDot"),
  cameraSignalText: byId("cameraSignalText"),
  cameraEndpointLabel: byId("cameraEndpointLabel"),
  latencyValue: byId("latencyValue"),
  flightTimeValue: byId("flightTimeValue"),
  headingValue: byId("headingValue"),
  rollValue: byId("rollValue"),
  pitchValue: byId("pitchValue"),
  altitudeValue: byId("altitudeValue"),
  speedValue: byId("speedValue"),
  voltageValue: byId("voltageValue"),
  currentValue: byId("currentValue"),
  satelliteValue: byId("satelliteValue"),
  commandAgeValue: byId("commandAgeValue"),
  // Flight safety controls and the authoritative state label.
  safetyState: byId("safetyState"),
  armHoldButton: byId("armHoldButton"),
  disarmButton: byId("disarmButton"),
  returnHomeButton: byId("returnHomeButton"),
  calibrateImuButton: byId("calibrateImuButton"),
  // Separate payload-node controls never receive motor authority.
  payloadState: byId("payloadState"),
  snapshotButton: byId("snapshotButton"),
  recordButton: byId("recordButton"),
  audioButton: byId("audioButton"),
  speakerButton: byId("speakerButton"),
  cameraTilt: byId("cameraTilt"),
  cameraTiltValue: byId("cameraTiltValue"),
  fullscreenButton: byId("fullscreenButton"),
  // Virtual pilot sticks and their numeric readouts.
  leftStick: byId("leftStick"),
  leftStickKnob: byId("leftStickKnob"),
  leftStickReadout: byId("leftStickReadout"),
  rightStick: byId("rightStick"),
  rightStickKnob: byId("rightStickKnob"),
  rightStickReadout: byId("rightStickReadout"),
  streamMode: byId("streamMode"),
  frameInfo: byId("frameInfo"),
  coordinateValue: byId("coordinateValue"),
  sensorGateValue: byId("sensorGateValue"),
  // Local mission-planning elements remain locked from upload until navigation is verified.
  missionWorkspace: byId("missionWorkspace"),
  missionMap: byId("missionMap"),
  missionPath: byId("missionPath"),
  waypointList: byId("waypointList"),
  saveMissionButton: byId("saveMissionButton"),
  clearMissionButton: byId("clearMissionButton"),
  uploadMissionButton: byId("uploadMissionButton"),
  // Lower telemetry strip and event log elements.
  throttleStrip: byId("throttleStrip"),
  throttleMeter: byId("throttleMeter"),
  powerValue: byId("powerValue"),
  modeValue: byId("modeValue"),
  flowValue: byId("flowValue"),
  obstacleValue: byId("obstacleValue"),
  temperatureValue: byId("temperatureValue"),
  humidityValue: byId("humidityValue"),
  eventLog: byId("eventLog"),
  emergencyButton: byId("emergencyButton"),
  // Connection and camera endpoints are edited through this modal settings form.
  settingsDialog: byId("settingsDialog"),
  settingsForm: byId("settingsForm"),
  closeSettings: byId("closeSettings"),
  cancelSettings: byId("cancelSettings"),
  flightHost: byId("flightHost"),
  cameraUrl: byId("cameraUrl"),
  cameraApiUrl: byId("cameraApiUrl"),
};

// Load saved endpoints defensively because localStorage may contain invalid JSON.
const savedSettings = (() => {
  try {
    return JSON.parse(localStorage.getItem(SETTINGS_KEY) || "{}");
  } catch (_error) {
    return {};
  }
})();

// Keep all mutable interface and protocol state in one explicit object.
const state = {
  // WebSocket lifecycle, timers, packet sequences, and measured latency.
  socket: null,
  connected: false,
  connecting: false,
  commandTimer: null,
  uiTimer: null,
  payloadTimer: null,
  nextSequence: 1,
  pendingCommands: new Map(),
  latencyMs: null,
  lastTelemetryAt: 0,
  // Operator-configurable endpoints fall back to safe NanoKit defaults.
  flightHost: savedSettings.flightHost || DEFAULT_FLIGHT_HOST,
  cameraUrl: savedSettings.cameraUrl || DEFAULT_CAMERA_URL,
  cameraApiUrl: savedSettings.cameraApiUrl || DEFAULT_CAMERA_API_URL,
  // Camera-node readiness is reported independently for each payload function.
  cameraLive: false,
  payload: {
    cameraReady: false,
    audioCaptureReady: false,
    audioPlaybackReady: false,
    storageReady: false,
    psram8MB: false,
  },
  // Local view and payload interaction state never grants flight authority.
  recording: false,
  audioMonitoring: false,
  speakerActive: false,
  view: "standard",
  flightMode: "MANUAL",
  // Arming and one-shot action timers control the command flags sent to firmware.
  armRequested: false,
  armed: false,
  armedSince: 0,
  accumulatedFlightMs: 0,
  stopPulseUntil: 0,
  calibrationPulseUntil: 0,
  armHoldTimer: null,
  // Pilot commands use normalized throttle and engineering units for all axes.
  control: { throttle: 0, roll: 0, pitch: 0, yaw: 0 },
  // The newest telemetry packet and its explicit validity gates are retained together.
  telemetry: {},
  safety: {
    imuHealthy: false,
    imuCalibrated: false,
    attitudeValid: false,
    escConfirmed: false,
    gnssFix: false,
    barometerValid: false,
    environmentValid: false,
    powerValid: false,
    navigationReady: false,
  },
  // Waypoints are local planning data until navigation hardware is validated.
  waypoints: [],
};

/** Parse one semicolon-separated firmware packet into a key/value object. */
function parsePacket(payload) {
  const fields = {};
  // Ignore malformed fragments so one bad field cannot break the full interface.
  String(payload).split(";").forEach((part) => {
    const separator = part.indexOf("=");
    if (separator <= 0) return;
    fields[part.slice(0, separator).trim()] = part.slice(separator + 1).trim();
  });
  return fields;
}

/** Show the newest operator or protocol event in the persistent event log. */
function setEvent(message) {
  elements.eventLog.textContent = message;
}

/** Present a visible safety alert using one controlled severity style. */
function setAlert(level, title, message) {
  elements.alertBar.hidden = false;
  elements.alertBar.classList.remove("warning", "critical", "good");
  elements.alertBar.classList.add(level);
  elements.alertTitle.textContent = title;
  elements.alertMessage.textContent = message;
}

/** Replace a status indicator's severity class without changing its element. */
function setStatusDot(element, level) {
  element.classList.remove("good", "warning", "critical");
  element.classList.add(level);
}

/** Convert a host, URL, or address string into only the hostname used by WebSocket. */
function normalizeHost(value) {
  const raw = String(value || "").trim();
  if (!raw) return DEFAULT_FLIGHT_HOST;
  // Prefer standards-based URL parsing when the browser can understand the input.
  try {
    const parsed = new URL(raw.includes("://") ? raw : `http://${raw}`);
    return parsed.hostname || DEFAULT_FLIGHT_HOST;
  } catch (_error) {
    // Fall back to simple prefix, path, and port removal for partial user input.
    return raw.replace(/^(https?|wss?):\/\//i, "").split("/")[0].split(":")[0];
  }
}

/** Build the fixed unencrypted local WebSocket endpoint used by the NanoKit SoftAP. */
function websocketUrl() {
  return `ws://${normalizeHost(state.flightHost)}:81/`;
}

/** Update connection state, controls, status indicators, and the operator event together. */
function setLinkState(connected, detail = "") {
  state.connected = connected;
  state.connecting = false;
  elements.linkText.textContent = connected ? "LINKED" : "OFFLINE";
  setStatusDot(elements.linkDot, connected ? "good" : "critical");
  elements.connectButton.textContent = connected ? "Disconnect" : "Connect";
  elements.connectButton.disabled = false;
  setEvent(detail || (connected ? "WebSocket command link active" : "Flight link offline"));
  // Re-evaluate arming because connection state is one of its mandatory gates.
  updateSafetyControls();
}

/** Format a finite numeric field or return null for missing and invalid telemetry. */
function formatNumber(value, digits = 1) {
  const number = Number(value);
  return Number.isFinite(number) ? number.toFixed(digits) : null;
}

/** Treat only the protocol value "1" as an enabled hardware validity flag. */
function fieldEnabled(fields, key) {
  return fields[key] === "1";
}

/** Enable actions only when the current link, pilot, and firmware safety gates allow them. */
function updateSafetyControls() {
  // Centered attitude controls are required before the interface permits arming.
  const controlsCentered = Math.abs(state.control.roll) < 0.01 &&
    Math.abs(state.control.pitch) < 0.01 && Math.abs(state.control.yaw) < 0.01;
  const canArm = state.connected && !state.armed && state.control.throttle === 0 &&
    controlsCentered && state.safety.imuHealthy && state.safety.imuCalibrated &&
    state.safety.attitudeValid && state.safety.escConfirmed;

  // Arm, disarm, calibration, return-home, and mission controls have independent gates.
  elements.armHoldButton.disabled = !canArm;
  elements.armHoldButton.classList.toggle("armed", state.armed);
  elements.armHoldButton.querySelector("b").textContent = state.armed ? "SYSTEM ARMED" : "HOLD TO ARM";
  elements.armHoldButton.querySelector("small").textContent = state.armed ? "Live control authority" : "1.5 seconds";
  elements.disarmButton.disabled = !state.connected || !state.armed;
  elements.calibrateImuButton.disabled = !state.connected || !state.safety.imuHealthy || state.armed;
  elements.returnHomeButton.disabled = !state.connected || !state.armed ||
    !state.safety.gnssFix || !state.safety.navigationReady;
  elements.uploadMissionButton.disabled = !state.connected || !state.safety.navigationReady ||
    state.waypoints.length === 0;

  // Firmware telemetry remains the authoritative visible safety state.
  const safetyName = state.telemetry.STATE || (state.connected ? "DISARMED" : "LOCKED");
  elements.safetyState.textContent = safetyName;
  elements.armTop.textContent = state.armed ? "ARMED" : safetyName;
  elements.sensorGateValue.textContent = state.safety.imuHealthy
    ? (state.safety.imuCalibrated ? "IMU READY" : "CALIBRATION")
    : "IMU LOCK";
}

/** Render pilot command values and virtual-stick positions without sending a packet. */
function updateControlDisplay() {
  // Convert normalized 0-1000 throttle into a display percentage.
  const throttlePercent = Math.round(state.control.throttle / 10);
  elements.leftStickReadout.textContent = `T ${throttlePercent} / Y ${Math.round(state.control.yaw)}`;
  elements.rightStickReadout.textContent = `P ${Math.round(state.control.pitch)} / R ${Math.round(state.control.roll)}`;
  elements.throttleStrip.textContent = `${throttlePercent}%`;
  elements.throttleMeter.style.width = `${throttlePercent}%`;
  elements.leftStick.style.setProperty("--stick-x", `${50 + (state.control.yaw / 120) * 50}%`);
  elements.leftStick.style.setProperty("--stick-y", `${100 - throttlePercent}%`);
  elements.rightStick.style.setProperty("--stick-x", `${50 + (state.control.roll / 25) * 50}%`);
  elements.rightStick.style.setProperty("--stick-y", `${50 - (state.control.pitch / 25) * 50}%`);
  // Changing stick values can change whether the system is allowed to arm.
  updateSafetyControls();
}

/** Center both sticks and optionally force throttle to zero. */
function resetPilotControls(resetThrottle = true) {
  if (resetThrottle) state.control.throttle = 0;
  state.control.roll = 0;
  state.control.pitch = 0;
  state.control.yaw = 0;
  updateControlDisplay();
}

/** Serialize the current pilot state into one versioned command packet. */
function buildCommand() {
  // A strictly increasing sequence supports acknowledgement and latency tracking.
  const sequence = state.nextSequence++;
  // Disarmed commands always transmit zero throttle even if the UI retained a value.
  const packet = [
    `PROTO=${PROTOCOL_VERSION}`,
    "TYPE=CMD",
    `SEQ=${sequence}`,
    `T=${state.armed ? state.control.throttle : 0}`,
    `R=${state.control.roll.toFixed(2)}`,
    `P=${state.control.pitch.toFixed(2)}`,
    `Y=${state.control.yaw.toFixed(2)}`,
    `A=${state.armRequested ? 1 : 0}`,
    `CAL=${Date.now() < state.calibrationPulseUntil ? 1 : 0}`,
    `STOP=${Date.now() < state.stopPulseUntil ? 1 : 0}`,
    `MODE=${state.flightMode}`,
    "",
  ].join(";");
  // Return both forms so the sender can track the exact sequence it transmits.
  return { sequence, packet };
}

/** Send one command when the WebSocket is open and optionally measure acknowledgement latency. */
function sendCommand(trackLatency = true) {
  // Never queue commands while disconnected or while the socket is not fully open.
  if (!state.connected || !state.socket || state.socket.readyState !== WebSocket.OPEN) return false;
  const command = buildCommand();
  try {
    state.socket.send(command.packet);
    // Keep the original send time until firmware acknowledges this sequence.
    if (trackLatency) state.pendingCommands.set(command.sequence, performance.now());
    // Bound the map so a missing acknowledgement cannot grow memory indefinitely.
    while (state.pendingCommands.size > 30) {
      state.pendingCommands.delete(state.pendingCommands.keys().next().value);
    }
    return true;
  } catch (error) {
    // A write exception is visible and does not pretend the command succeeded.
    setAlert("critical", "COMMAND FAILED", error.message || "WebSocket write failed.");
    return false;
  }
}

/** Start the periodic command heartbeat required by the firmware freshness failsafe. */
function startCommandStream() {
  clearInterval(state.commandTimer);
  sendCommand();
  state.commandTimer = setInterval(() => sendCommand(), COMMAND_PERIOD_MS);
}

/** Stop all periodic command transmission immediately. */
function stopCommandStream() {
  clearInterval(state.commandTimer);
  state.commandTimer = null;
}

/** End the command link, clear arming intent, and return controls to safe values. */
function disconnectFlightLink(reason = "Disconnected by operator") {
  stopCommandStream();
  state.armRequested = false;
  state.armed = false;
  resetPilotControls(true);
  const socket = state.socket;
  // Clear the active reference before close events run so stale callbacks are ignored.
  state.socket = null;
  if (socket && socket.readyState < WebSocket.CLOSING) socket.close(1000, reason);
  setLinkState(false, reason);
}

/** Create and manage a direct WebSocket connection to the flight controller. */
function connectFlightLink() {
  // The same button disconnects an existing or pending connection.
  if (state.connected || state.connecting) {
    disconnectFlightLink();
    return;
  }
  // Browsers block insecure ws:// from an HTTPS page, so explain the required local URL.
  if (location.protocol === "https:") {
    setAlert("critical", "BROWSER SECURITY BLOCK", "Open http://192.168.4.1 on the NanoKit network. HTTPS pages cannot open the controller's plain WebSocket.");
    return;
  }

  // Lock the connect control while the browser creates the new socket.
  const url = websocketUrl();
  state.connecting = true;
  elements.connectButton.disabled = true;
  elements.connectButton.textContent = "Connecting";
  setEvent(`Opening ${url}`);

  let socket;
  // WebSocket construction can fail immediately for an invalid endpoint.
  try {
    socket = new WebSocket(url);
  } catch (error) {
    state.connecting = false;
    elements.connectButton.disabled = false;
    setAlert("critical", "LINK ERROR", error.message || "Unable to create WebSocket.");
    return;
  }
  state.socket = socket;

  // Start the command heartbeat only after the browser confirms an open connection.
  socket.addEventListener("open", () => {
    if (state.socket !== socket) return;
    state.lastTelemetryAt = Date.now();
    setLinkState(true, `WebSocket active at ${url}`);
    setAlert("warning", "SAFETY GATES ACTIVE", "The controller is connected. Arming remains blocked until IMU, calibration, and ESC protocol checks pass.");
    startCommandStream();
  });

  // Route protocol data and lifecycle failures to their dedicated handlers.
  socket.addEventListener("message", (event) => handleFlightPacket(event.data));
  socket.addEventListener("error", () => {
    setAlert("critical", "LINK ERROR", "The Flight Deck could not maintain the WebSocket connection.");
  });
  socket.addEventListener("close", (event) => {
    if (state.socket !== socket) return;
    state.socket = null;
    // A closed flight link always removes arm intent and pilot demand locally.
    stopCommandStream();
    state.armRequested = false;
    state.armed = false;
    resetPilotControls(true);
    setLinkState(false, `WebSocket closed (${event.code})`);
    setAlert("critical", "FLIGHT LINK LOST", "Commands stopped. Firmware failsafe forces all motors to minimum.");
  });
}

/** Resolve one acknowledged command into a round-trip latency measurement. */
function updateLatency(sequence) {
  const sentAt = state.pendingCommands.get(Number(sequence));
  if (sentAt === undefined) return;
  // performance.now() provides a monotonic high-resolution browser timer.
  state.latencyMs = Math.max(0, Math.round(performance.now() - sentAt));
  state.pendingCommands.delete(Number(sequence));
  elements.latencyValue.textContent = `${state.latencyMs} ms`;
}

/** Validate and apply one HELLO, ACK, ERROR, or telemetry packet from firmware. */
function handleFlightPacket(payload) {
  const fields = parsePacket(payload);
  // Refuse incompatible packet layouts instead of rendering misleading values.
  if (fields.PROTO !== PROTOCOL_VERSION) {
    setAlert("critical", "PROTOCOL MISMATCH", "Upload the current NanoKit Drone 4X Wi-Fi firmware.");
    return;
  }

  // HELLO confirms device identity but carries no flight telemetry.
  if (fields.TYPE === "HELLO") {
    setEvent(`${fields.DEVICE || "NanoKit-Drone-4X"} Wi-Fi link confirmed`);
    return;
  }
  // ACK closes the latency measurement for one sent command.
  if (fields.TYPE === "ACK") {
    updateLatency(fields.SEQ);
    return;
  }
  // Firmware validation errors remain visible to the operator.
  if (fields.TYPE === "ERROR") {
    setAlert("critical", fields.CODE || "COMMAND ERROR", fields.MESSAGE || "Firmware rejected the command.");
    return;
  }
  // Ignore unknown packet types rather than interpreting them as telemetry.
  if (fields.TYPE !== "TEL") return;

  // Store the complete newest telemetry packet and any included acknowledgement.
  state.lastTelemetryAt = Date.now();
  state.telemetry = fields;
  if (fields.ACK) updateLatency(fields.ACK);

  // Track armed duration and clear pilot controls immediately after disarming.
  const wasArmed = state.armed;
  state.armed = fields.ARM === "1";
  if (!wasArmed && state.armed) state.armedSince = Date.now();
  if (wasArmed && !state.armed && state.armedSince) {
    state.accumulatedFlightMs += Date.now() - state.armedSince;
    state.armedSince = 0;
    state.armRequested = false;
    resetPilotControls(true);
  }

  // Copy explicit firmware validity bits before displaying any sensor number.
  state.safety.imuHealthy = fieldEnabled(fields, "IMU");
  state.safety.imuCalibrated = fieldEnabled(fields, "CAL");
  state.safety.attitudeValid = fieldEnabled(fields, "ATT_VALID");
  state.safety.escConfirmed = fieldEnabled(fields, "ESC_OK");
  state.safety.gnssFix = fieldEnabled(fields, "GNSS");
  state.safety.barometerValid = fieldEnabled(fields, "BARO");
  state.safety.environmentValid = fieldEnabled(fields, "ENV");
  state.safety.powerValid = fieldEnabled(fields, "POWER");
  state.safety.navigationReady = fieldEnabled(fields, "NAV");

  // Attitude values are shown only when both IMU health and fused attitude are valid.
  const attitudeValid = state.safety.imuHealthy && state.safety.attitudeValid;
  const roll = attitudeValid ? formatNumber(fields.ROLL) : null;
  const pitch = attitudeValid ? formatNumber(fields.PITCH) : null;
  const yaw = attitudeValid ? formatNumber(fields.YAW) : null;
  elements.rollValue.textContent = roll === null ? "--.- deg" : `${roll} deg`;
  elements.pitchValue.textContent = pitch === null ? "--.- deg" : `${pitch} deg`;
  elements.headingValue.textContent = yaw === null ? "--- deg" : `${yaw} deg`;

  // Barometer and GNSS values use their own independent hardware validity gates.
  const altitude = state.safety.barometerValid ? formatNumber(fields.ALT, 2) : null;
  elements.altitudeValue.textContent = altitude === null ? "--.- m" : `${altitude} m`;
  const speed = state.safety.gnssFix ? formatNumber(fields.SPD, 2) : null;
  elements.speedValue.textContent = speed === null ? "--.- m/s" : `${speed} m/s`;

  // Power telemetry is hidden as unavailable until a real power monitor is valid.
  const voltage = state.safety.powerValid ? formatNumber(fields.BAT_V, 2) : null;
  const current = state.safety.powerValid ? formatNumber(fields.BAT_A, 2) : null;
  const power = state.safety.powerValid ? formatNumber(fields.BAT_W, 1) : null;
  elements.voltageValue.textContent = voltage === null ? "--.- V" : `${voltage} V`;
  elements.currentValue.textContent = current === null ? "--.- A" : `${current} A`;
  elements.powerValue.textContent = power === null ? "--.- W" : `${power} W`;
  elements.batteryTop.textContent = voltage === null ? "--.- V" : `${voltage} V`;

  // GNSS position, command age, mode, optical flow, and obstacles update independently.
  elements.gnssTop.textContent = state.safety.gnssFix ? "FIX" : "NO FIX";
  elements.satelliteValue.textContent = state.safety.gnssFix ? (fields.SAT || "0") : "--";
  elements.coordinateValue.textContent = state.safety.gnssFix
    ? `${formatNumber(fields.LAT, 7)}, ${formatNumber(fields.LON, 7)}`
    : "--.-------, --.-------";
  elements.commandAgeValue.textContent = fields.CMD_AGE ? `${fields.CMD_AGE} ms` : "-- ms";
  elements.modeValue.textContent = fields.MODE || state.flightMode;
  elements.flowValue.textContent = fieldEnabled(fields, "FLOW") ? "TRACKING" : "OFFLINE";
  elements.obstacleValue.textContent = fieldEnabled(fields, "OBST") ? "ACTIVE" : "OFFLINE";

  // Environmental values remain placeholders unless the firmware marks them valid.
  const temperature = state.safety.environmentValid ? formatNumber(fields.TEMP) : null;
  const humidity = state.safety.environmentValid ? formatNumber(fields.HUM) : null;
  elements.temperatureValue.textContent = temperature === null ? "--.- degC" : `${temperature} degC`;
  elements.humidityValue.textContent = humidity === null ? "--%" : `${humidity}%`;

  // Mirror telemetry throttle only while the firmware confirms an armed state.
  const telemetryThrottle = state.armed ? Number(fields.THR || 0) : 0;
  if (state.armed && Number.isFinite(telemetryThrottle)) state.control.throttle = clamp(telemetryThrottle, 0, 1000);
  updateControlDisplay();
  updateSafetyControls();
  // Present the firmware status after all supporting state has been updated.
  presentSafetyStatus(fields);
}

/** Translate authoritative firmware safety fields into one operator alert. */
function presentSafetyStatus(fields) {
  const status = fields.STATUS || "Safety state updated";
  // Fault and failsafe states always override lower-priority readiness messages.
  if (fields.FAILSAFE === "1" || fields.STATE === "FAILSAFE" || fields.STATE === "FAULT") {
    setAlert("critical", fields.STATE || "FAULT", status);
  } else if (state.armed) {
    setAlert("good", "AIRCRAFT ARMED", status);
  } else if (!state.safety.imuHealthy) {
    setAlert("critical", "IMU UNAVAILABLE", status);
  } else if (!state.safety.imuCalibrated) {
    setAlert("warning", "CALIBRATION REQUIRED", status);
  } else if (!state.safety.escConfirmed) {
    setAlert("warning", "ESC OUTPUT LOCKED", status);
  } else {
    setAlert("good", "READY TO ARM", status);
  }
  setEvent(status);
}

/** Begin the intentional hold-to-arm timer without sending an immediate arm request. */
function beginArmHold(event) {
  event.preventDefault();
  if (elements.armHoldButton.disabled || state.armHoldTimer) return;
  elements.armHoldButton.classList.add("holding");
  // The delayed callback sends Arm only after the full safety hold duration.
  state.armHoldTimer = setTimeout(() => {
    state.armHoldTimer = null;
    elements.armHoldButton.classList.remove("holding");
    state.armRequested = true;
    sendCommand(false);
    setEvent("ARM requested; waiting for firmware safety approval");
  }, ARM_HOLD_MS);
}

/** Cancel a partial hold so a short click or pointer exit cannot arm the aircraft. */
function cancelArmHold() {
  clearTimeout(state.armHoldTimer);
  state.armHoldTimer = null;
  elements.armHoldButton.classList.remove("holding");
}

/** Clear arm intent, zero controls, and send a normal disarm command. */
function disarm() {
  cancelArmHold();
  state.armRequested = false;
  resetPilotControls(true);
  sendCommand(false);
  setEvent("DISARM command sent");
}

/** Send the one-shot emergency flag and force all local pilot values to zero. */
function emergencyStop() {
  cancelArmHold();
  state.armRequested = false;
  // Keep STOP high for several periodic packets so brief network timing cannot miss it.
  state.stopPulseUntil = Date.now() + 350;
  resetPilotControls(true);
  sendCommand(false);
  setAlert("critical", "EMERGENCY STOP", "Emergency stop sent. Firmware latches the arm inhibit and forces every motor to minimum.");
}

/** Send a short calibration flag only when firmware-reported gates permit it. */
function requestCalibration() {
  if (elements.calibrateImuButton.disabled) return;
  state.calibrationPulseUntil = Date.now() + 350;
  sendCommand(false);
  setEvent("IMU calibration requested; keep the frame level and motionless");
}

/** Bind one pointer-controlled virtual stick to the appropriate pilot axes. */
function bindStick(zone, side) {
  // Track one active pointer so unrelated touch or mouse events are ignored.
  let pointerId = null;

  // Convert pointer position into normalized throttle/yaw or roll/pitch commands.
  const updateFromPointer = (event) => {
    if (!state.connected || event.pointerId !== pointerId) return;
    const rect = zone.getBoundingClientRect();
    const x = clamp((event.clientX - rect.left) / rect.width, 0, 1);
    const y = clamp((event.clientY - rect.top) / rect.height, 0, 1);
    // The left stick controls yaw and armed throttle; the right controls attitude.
    if (side === "left") {
      state.control.yaw = (x * 2 - 1) * 120;
      if (state.armed) state.control.throttle = Math.round((1 - y) * 1000);
    } else {
      state.control.roll = (x * 2 - 1) * 25;
      state.control.pitch = (1 - y * 2) * 25;
    }
    updateControlDisplay();
  };

  // Capture the pointer so control remains stable if it moves outside the stick element.
  zone.addEventListener("pointerdown", (event) => {
    if (!state.connected) {
      setAlert("warning", "NO FLIGHT LINK", "Connect the WebSocket command link before using the sticks.");
      return;
    }
    pointerId = event.pointerId;
    zone.setPointerCapture(pointerId);
    zone.classList.add("dragging");
    updateFromPointer(event);
  });
  zone.addEventListener("pointermove", updateFromPointer);
  // Releasing either stick centers its attitude axes; throttle remains where commanded.
  const release = (event) => {
    if (event.pointerId !== pointerId) return;
    zone.classList.remove("dragging");
    pointerId = null;
    if (side === "left") state.control.yaw = 0;
    else {
      state.control.roll = 0;
      state.control.pitch = 0;
    }
    updateControlDisplay();
  };
  zone.addEventListener("pointerup", release);
  zone.addEventListener("pointercancel", release);
}

/** Switch between standard, FPV, camera, and local mission interface views. */
function switchView(view) {
  state.view = view;
  document.querySelectorAll(".mode-button").forEach((button) => {
    button.classList.toggle("active", button.dataset.view === view);
  });
  // CSS classes control presentation while hidden flags select stage or mission content.
  elements.flightDeck.classList.toggle("fpv-mode", view === "fpv");
  elements.flightDeck.classList.toggle("camera-mode", view === "camera");
  elements.videoStage.hidden = view === "mission";
  elements.missionWorkspace.hidden = view !== "mission";
}

/** Select a requested flight mode and let firmware perform final mode validation. */
function chooseFlightMode(mode) {
  state.flightMode = mode;
  document.querySelectorAll(".flight-mode").forEach((button) => {
    button.classList.toggle("active", button.dataset.flightMode === mode);
  });
  elements.modeValue.textContent = mode;
  // Send immediately so the operator does not wait for the next heartbeat.
  sendCommand(false);
}

/** Toggle camera-stage presentation while preserving an explicit status message. */
function setCameraLive(live, message) {
  state.cameraLive = live;
  elements.cameraFeed.hidden = !live;
  elements.offlineReference.hidden = live;
  setStatusDot(elements.cameraSignalDot, live ? "good" : "warning");
  elements.cameraSignalText.textContent = live ? "CAMERA LIVE" : "CAMERA STANDBY";
  elements.streamMode.textContent = live ? "LIVE CAMERA" : "REFERENCE VIEW";
  elements.frameInfo.textContent = message;
}

/** Load the configured observation stream with a cache-busting query value. */
function loadCameraStream() {
  if (!state.cameraUrl) {
    setCameraLive(false, "Camera stream URL is not configured");
    return;
  }
  elements.cameraEndpointLabel.textContent = state.cameraUrl;
  // Browser image load events are the source of truth for visible stream status.
  elements.cameraFeed.onload = () => setCameraLive(true, "Observation stream online");
  elements.cameraFeed.onerror = () => setCameraLive(false, "Camera endpoint unavailable");
  elements.cameraFeed.src = `${state.cameraUrl}${state.cameraUrl.includes("?") ? "&" : "?"}t=${Date.now()}`;
}

/** Poll the separate camera node and update its independent hardware gates. */
async function refreshPayloadStatus() {
  if (!state.cameraApiUrl) return;
  // Abort a slow payload request so it cannot accumulate behind later polls.
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), 2500);
  try {
    // Disable browser caching because readiness can change after a hardware reboot.
    const response = await fetch(`${state.cameraApiUrl.replace(/\/$/, "")}/api/status`, {
      signal: controller.signal,
      cache: "no-store",
    });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    // Accept only strict JSON booleans to avoid treating text as hardware readiness.
    const payload = await response.json();
    state.payload.cameraReady = payload.cameraReady === true;
    state.payload.audioCaptureReady = payload.audioCaptureReady === true;
    state.payload.audioPlaybackReady = payload.audioPlaybackReady === true;
    state.payload.storageReady = payload.storageReady === true;
    state.payload.psram8MB = payload.psram8MB === true;
    elements.payloadState.textContent = state.payload.cameraReady ? "READY" : "LOCKED";
  } catch (_error) {
    // Any timeout, network, or parse failure returns every payload control to offline.
    Object.keys(state.payload).forEach((key) => { state.payload[key] = false; });
    elements.payloadState.textContent = "OFFLINE";
  } finally {
    // Always clear the timer and re-evaluate payload controls after the request finishes.
    clearTimeout(timeout);
    updatePayloadControls();
  }
}

/** Enable each payload button only when its exact hardware dependency is ready. */
function updatePayloadControls() {
  elements.snapshotButton.disabled = !state.payload.cameraReady;
  elements.recordButton.disabled = !(state.payload.cameraReady && state.payload.storageReady);
  elements.audioButton.disabled = !state.payload.audioCaptureReady;
  elements.speakerButton.disabled = !state.payload.audioPlaybackReady;
  elements.cameraTilt.disabled = !state.payload.cameraReady;
}

/** Send one JSON command to the camera node and surface any rejection. */
async function postPayload(path, body) {
  try {
    // Payload endpoints are separate HTTP requests and cannot control flight motors.
    const response = await fetch(`${state.cameraApiUrl.replace(/\/$/, "")}${path}`, {
      method: "POST",
      headers: { "Content-Type": "text/plain" },
      body: JSON.stringify(body),
    });
    const payload = await response.json().catch(() => ({}));
    // Non-success responses use the node's reason when one is available.
    if (!response.ok) throw new Error(payload.reason || `HTTP ${response.status}`);
    return payload;
  } catch (error) {
    setAlert("warning", "PAYLOAD COMMAND REJECTED", error.message || "Camera node unavailable.");
    return null;
  }
}

/** Restore up to fifty local planning waypoints from browser storage. */
function readStoredMission() {
  try {
    const stored = JSON.parse(localStorage.getItem(MISSION_KEY) || "[]");
    // The limit bounds rendering work and the size of locally saved mission data.
    if (Array.isArray(stored)) state.waypoints = stored.slice(0, 50);
  } catch (_error) {
    state.waypoints = [];
  }
}

/** Render the local mission path, numbered markers, and readable waypoint list. */
function renderMission() {
  // Rebuild the SVG from state so deleted or reordered points cannot leave stale nodes.
  elements.missionPath.replaceChildren();
  const namespace = "http://www.w3.org/2000/svg";
  if (state.waypoints.length > 1) {
    // Draw one continuous polyline through all staged points.
    const path = document.createElementNS(namespace, "polyline");
    path.setAttribute("points", state.waypoints.map((point) => `${point.x * 10},${point.y * 6}`).join(" "));
    path.setAttribute("fill", "none");
    path.setAttribute("stroke", "#72d6ef");
    path.setAttribute("stroke-width", "3");
    path.setAttribute("vector-effect", "non-scaling-stroke");
    elements.missionPath.append(path);
  }
  // Add one numbered SVG circle for every waypoint.
  state.waypoints.forEach((point, index) => {
    const marker = document.createElementNS(namespace, "circle");
    marker.setAttribute("cx", String(point.x * 10));
    marker.setAttribute("cy", String(point.y * 6));
    marker.setAttribute("r", "8");
    marker.setAttribute("fill", "#070b0e");
    marker.setAttribute("stroke", "#a8df62");
    marker.setAttribute("stroke-width", "3");
    marker.setAttribute("vector-effect", "non-scaling-stroke");
    elements.missionPath.append(marker);
    const label = document.createElementNS(namespace, "text");
    label.setAttribute("x", String(point.x * 10));
    label.setAttribute("y", String(point.y * 6 + 3));
    label.setAttribute("text-anchor", "middle");
    label.setAttribute("fill", "#edf5f7");
    label.setAttribute("font-size", "9");
    label.textContent = String(index + 1);
    elements.missionPath.append(label);
  });

  // Rebuild the text list to match the same source waypoint array.
  elements.waypointList.replaceChildren();
  if (state.waypoints.length === 0) {
    const empty = document.createElement("li");
    empty.className = "empty-waypoints";
    empty.textContent = "No waypoints staged";
    elements.waypointList.append(empty);
  } else {
    state.waypoints.forEach((point, index) => {
      const item = document.createElement("li");
      item.textContent = `WP${index + 1}  X ${point.x.toFixed(1)}  Y ${point.y.toFixed(1)}`;
      elements.waypointList.append(item);
    });
  }
  // Persist local planning only; this does not upload a mission to the aircraft.
  localStorage.setItem(MISSION_KEY, JSON.stringify(state.waypoints));
  updateSafetyControls();
}

/** Download the staged local mission as a clearly marked non-uploaded JSON file. */
function saveMissionFile() {
  const documentBody = {
    format: "nanokit-drone-4x-local-mission",
    version: 1,
    uploaded: false,
    note: "Local planning only until GNSS and navigation feature gates are verified.",
    waypoints: state.waypoints,
  };
  // A temporary object URL lets the browser download generated JSON without a server.
  const blob = new Blob([JSON.stringify(documentBody, null, 2)], { type: "application/json" });
  const link = document.createElement("a");
  link.href = URL.createObjectURL(blob);
  link.download = "nanokit-drone-4x-mission.json";
  link.click();
  // Release the temporary URL immediately after starting the download.
  URL.revokeObjectURL(link.href);
}

/** Update armed flight time and warn when a connected link stops receiving telemetry. */
function updateClocks() {
  const liveFlightMs = state.armed && state.armedSince ? Date.now() - state.armedSince : 0;
  const totalSeconds = Math.floor((state.accumulatedFlightMs + liveFlightMs) / 1000);
  const minutes = String(Math.floor(totalSeconds / 60)).padStart(2, "0");
  const seconds = String(totalSeconds % 60).padStart(2, "0");
  elements.flightTimeValue.textContent = `${minutes}:${seconds}`;
  // A stale display warns the operator before the firmware command timeout is forgotten.
  if (state.connected && Date.now() - state.lastTelemetryAt > 1200) {
    setStatusDot(elements.linkDot, "warning");
    elements.linkText.textContent = "STALE";
  }
}

/** Populate and open the endpoint settings dialog. */
function openSettings() {
  elements.flightHost.value = state.flightHost;
  elements.cameraUrl.value = state.cameraUrl;
  elements.cameraApiUrl.value = state.cameraApiUrl;
  elements.settingsDialog.showModal();
}

/** Validate, persist, and apply operator endpoint settings. */
function saveSettings() {
  // Remember the previous host so an active link can be closed when the endpoint changes.
  const previousHost = normalizeHost(state.flightHost);
  state.flightHost = normalizeHost(elements.flightHost.value);
  state.cameraUrl = elements.cameraUrl.value.trim();
  state.cameraApiUrl = elements.cameraApiUrl.value.trim().replace(/\/$/, "");
  localStorage.setItem(SETTINGS_KEY, JSON.stringify({
    flightHost: state.flightHost,
    cameraUrl: state.cameraUrl,
    cameraApiUrl: state.cameraApiUrl,
  }));
  // Refresh both the flight link decision and separate payload endpoints.
  elements.cameraEndpointLabel.textContent = state.cameraUrl || "No stream endpoint";
  if (state.connected && previousHost !== state.flightHost) disconnectFlightLink("Flight host changed");
  loadCameraStream();
  refreshPayloadStatus();
}

// Connect top-level flight, settings, alert, arming, and emergency controls.
elements.connectButton.addEventListener("click", connectFlightLink);
elements.settingsButton.addEventListener("click", openSettings);
elements.dismissAlert.addEventListener("click", () => { elements.alertBar.hidden = true; });
elements.armHoldButton.addEventListener("pointerdown", beginArmHold);
elements.armHoldButton.addEventListener("pointerup", cancelArmHold);
elements.armHoldButton.addEventListener("pointerleave", cancelArmHold);
elements.armHoldButton.addEventListener("pointercancel", cancelArmHold);
elements.disarmButton.addEventListener("click", disarm);
elements.calibrateImuButton.addEventListener("click", requestCalibration);
elements.emergencyButton.addEventListener("click", emergencyStop);
// Fullscreen is limited to the video stage and reports browser rejection in the event log.
elements.fullscreenButton.addEventListener("click", () => {
  if (document.fullscreenElement) document.exitFullscreen();
  else elements.videoStage.requestFullscreen().catch((error) => setEvent(error.message));
});

// View buttons change presentation only; flight-mode buttons send a validated request.
document.querySelectorAll(".mode-button").forEach((button) => {
  button.addEventListener("click", () => switchView(button.dataset.view));
});
document.querySelectorAll(".flight-mode").forEach((button) => {
  button.addEventListener("click", () => chooseFlightMode(button.dataset.flightMode));
});

// Bind both virtual sticks after their elements and shared state are available.
bindStick(elements.leftStick, "left");
bindStick(elements.rightStick, "right");

// Payload controls call only the separate camera-node API.
elements.snapshotButton.addEventListener("click", () => postPayload("/api/camera", { action: "snapshot" }));
elements.recordButton.addEventListener("click", async () => {
  // Update local recording presentation only after the node accepts the request.
  const next = !state.recording;
  if (await postPayload("/api/recording", { action: next ? "start" : "stop" })) {
    state.recording = next;
    elements.recordButton.classList.toggle("active", next);
    elements.recordButton.lastChild.textContent = next ? " Stop" : " Record";
  }
});
elements.audioButton.addEventListener("click", async () => {
  // Microphone monitoring and speaker output use independent readiness gates.
  const next = !state.audioMonitoring;
  if (await postPayload("/api/audio", { action: next ? "monitor-on" : "monitor-off" })) {
    state.audioMonitoring = next;
    elements.audioButton.classList.toggle("active", next);
  }
});
elements.speakerButton.addEventListener("click", async () => {
  const next = !state.speakerActive;
  if (await postPayload("/api/audio", { action: next ? "speaker-on" : "speaker-off" })) {
    state.speakerActive = next;
    elements.speakerButton.classList.toggle("active", next);
  }
});
elements.cameraTilt.addEventListener("input", () => {
  // Input updates the preview label while change sends the final servo request.
  elements.cameraTiltValue.textContent = `${elements.cameraTilt.value} deg`;
});
elements.cameraTilt.addEventListener("change", () => {
  postPayload("/api/servo", { angle: Number(elements.cameraTilt.value) });
});

// Clicking the local map stages a bounded waypoint but never uploads it automatically.
elements.missionMap.addEventListener("click", (event) => {
  if (event.target.closest("p") || state.waypoints.length >= 50) return;
  // Store percentage coordinates so the plan remains responsive across screen sizes.
  const rect = elements.missionMap.getBoundingClientRect();
  state.waypoints.push({
    x: clamp(((event.clientX - rect.left) / rect.width) * 100, 0, 100),
    y: clamp(((event.clientY - rect.top) / rect.height) * 100, 0, 100),
  });
  renderMission();
});
// Clear and save actions affect local mission data only.
elements.clearMissionButton.addEventListener("click", () => {
  state.waypoints = [];
  renderMission();
});
elements.saveMissionButton.addEventListener("click", saveMissionFile);
elements.uploadMissionButton.addEventListener("click", () => {
  // Keep upload visibly locked until GNSS and navigation feature gates are verified.
  setAlert("warning", "NAVIGATION LOCKED", "Mission upload stays disabled until GNSS and navigation hardware are verified.");
});

// The settings form applies values while close and cancel only dismiss the dialog.
elements.settingsForm.addEventListener("submit", () => saveSettings());
elements.closeSettings.addEventListener("click", () => elements.settingsDialog.close());
elements.cancelSettings.addEventListener("click", () => elements.settingsDialog.close());

// Escape is an emergency shortcut only when it is not being used to close settings.
document.addEventListener("keydown", (event) => {
  if (event.key === "Escape" && !elements.settingsDialog.open) emergencyStop();
});
// Hiding the page while connected requests disarming as an additional operator safeguard.
document.addEventListener("visibilitychange", () => {
  if (document.hidden && state.connected) disarm();
});
// A final best-effort zero-throttle packet is sent before the browser unloads.
window.addEventListener("beforeunload", () => {
  state.armRequested = false;
  state.control.throttle = 0;
  sendCommand(false);
});

// Initialize local mission, display, camera, periodic status polling, and the locked state.
readStoredMission();
renderMission();
elements.cameraEndpointLabel.textContent = state.cameraUrl || "No stream endpoint";
updateControlDisplay();
updatePayloadControls();
switchView("standard");
loadCameraStream();
refreshPayloadStatus();
state.uiTimer = setInterval(updateClocks, 250);
// Payload readiness changes slowly, so a five-second poll avoids unnecessary requests.
state.payloadTimer = setInterval(refreshPayloadStatus, 5000);
setLinkState(false, "Flight Deck ready; Wi-Fi link not connected");
setAlert("warning", "SYSTEM LOCKED", "Connect to NanoKit-Drone-4X Wi-Fi, open http://192.168.4.1, then connect the WebSocket link.");
