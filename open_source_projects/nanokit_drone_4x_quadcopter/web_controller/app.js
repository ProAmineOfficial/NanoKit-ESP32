// NanoKit Drone 4X Wi-Fi Flight Deck.
// Developed by Amine Saoud ibn al-Bashir.
// Only validated telemetry is rendered. Missing sensors remain visibly offline.

"use strict";

const PROTOCOL_VERSION = "3";
const COMMAND_PERIOD_MS = 100;
const ARM_HOLD_MS = 1500;
const DEFAULT_FLIGHT_HOST = ["127.0.0.1", "localhost"].includes(location.hostname)
  ? "192.168.4.1"
  : location.hostname || "192.168.4.1";
const DEFAULT_CAMERA_URL = "http://nanokit-camera.local/stream";
const DEFAULT_CAMERA_API_URL = "http://nanokit-camera.local";
const SETTINGS_KEY = "nanokit-drone-4x-flight-deck-settings-v3";
const MISSION_KEY = "nanokit-drone-4x-local-mission";

const clamp = (value, minimum, maximum) => Math.max(minimum, Math.min(maximum, value));
const byId = (id) => document.getElementById(id);

const elements = {
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
  safetyState: byId("safetyState"),
  armHoldButton: byId("armHoldButton"),
  disarmButton: byId("disarmButton"),
  returnHomeButton: byId("returnHomeButton"),
  calibrateImuButton: byId("calibrateImuButton"),
  payloadState: byId("payloadState"),
  snapshotButton: byId("snapshotButton"),
  recordButton: byId("recordButton"),
  audioButton: byId("audioButton"),
  speakerButton: byId("speakerButton"),
  cameraTilt: byId("cameraTilt"),
  cameraTiltValue: byId("cameraTiltValue"),
  fullscreenButton: byId("fullscreenButton"),
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
  missionWorkspace: byId("missionWorkspace"),
  missionMap: byId("missionMap"),
  missionPath: byId("missionPath"),
  waypointList: byId("waypointList"),
  saveMissionButton: byId("saveMissionButton"),
  clearMissionButton: byId("clearMissionButton"),
  uploadMissionButton: byId("uploadMissionButton"),
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
  settingsDialog: byId("settingsDialog"),
  settingsForm: byId("settingsForm"),
  closeSettings: byId("closeSettings"),
  cancelSettings: byId("cancelSettings"),
  flightHost: byId("flightHost"),
  cameraUrl: byId("cameraUrl"),
  cameraApiUrl: byId("cameraApiUrl"),
};

const savedSettings = (() => {
  try {
    return JSON.parse(localStorage.getItem(SETTINGS_KEY) || "{}");
  } catch (_error) {
    return {};
  }
})();

const state = {
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
  flightHost: savedSettings.flightHost || DEFAULT_FLIGHT_HOST,
  cameraUrl: savedSettings.cameraUrl || DEFAULT_CAMERA_URL,
  cameraApiUrl: savedSettings.cameraApiUrl || DEFAULT_CAMERA_API_URL,
  cameraLive: false,
  payload: {
    cameraReady: false,
    audioCaptureReady: false,
    audioPlaybackReady: false,
    storageReady: false,
    psram8MB: false,
  },
  recording: false,
  audioMonitoring: false,
  speakerActive: false,
  view: "standard",
  flightMode: "MANUAL",
  armRequested: false,
  armed: false,
  armedSince: 0,
  accumulatedFlightMs: 0,
  stopPulseUntil: 0,
  calibrationPulseUntil: 0,
  armHoldTimer: null,
  control: { throttle: 0, roll: 0, pitch: 0, yaw: 0 },
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
  waypoints: [],
};

function parsePacket(payload) {
  const fields = {};
  String(payload).split(";").forEach((part) => {
    const separator = part.indexOf("=");
    if (separator <= 0) return;
    fields[part.slice(0, separator).trim()] = part.slice(separator + 1).trim();
  });
  return fields;
}

function setEvent(message) {
  elements.eventLog.textContent = message;
}

function setAlert(level, title, message) {
  elements.alertBar.hidden = false;
  elements.alertBar.classList.remove("warning", "critical", "good");
  elements.alertBar.classList.add(level);
  elements.alertTitle.textContent = title;
  elements.alertMessage.textContent = message;
}

function setStatusDot(element, level) {
  element.classList.remove("good", "warning", "critical");
  element.classList.add(level);
}

function normalizeHost(value) {
  const raw = String(value || "").trim();
  if (!raw) return DEFAULT_FLIGHT_HOST;
  try {
    const parsed = new URL(raw.includes("://") ? raw : `http://${raw}`);
    return parsed.hostname || DEFAULT_FLIGHT_HOST;
  } catch (_error) {
    return raw.replace(/^(https?|wss?):\/\//i, "").split("/")[0].split(":")[0];
  }
}

function websocketUrl() {
  return `ws://${normalizeHost(state.flightHost)}:81/`;
}

function setLinkState(connected, detail = "") {
  state.connected = connected;
  state.connecting = false;
  elements.linkText.textContent = connected ? "LINKED" : "OFFLINE";
  setStatusDot(elements.linkDot, connected ? "good" : "critical");
  elements.connectButton.textContent = connected ? "Disconnect" : "Connect";
  elements.connectButton.disabled = false;
  setEvent(detail || (connected ? "WebSocket command link active" : "Flight link offline"));
  updateSafetyControls();
}

function formatNumber(value, digits = 1) {
  const number = Number(value);
  return Number.isFinite(number) ? number.toFixed(digits) : null;
}

function fieldEnabled(fields, key) {
  return fields[key] === "1";
}

function updateSafetyControls() {
  const controlsCentered = Math.abs(state.control.roll) < 0.01 &&
    Math.abs(state.control.pitch) < 0.01 && Math.abs(state.control.yaw) < 0.01;
  const canArm = state.connected && !state.armed && state.control.throttle === 0 &&
    controlsCentered && state.safety.imuHealthy && state.safety.imuCalibrated &&
    state.safety.attitudeValid && state.safety.escConfirmed;

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

  const safetyName = state.telemetry.STATE || (state.connected ? "DISARMED" : "LOCKED");
  elements.safetyState.textContent = safetyName;
  elements.armTop.textContent = state.armed ? "ARMED" : safetyName;
  elements.sensorGateValue.textContent = state.safety.imuHealthy
    ? (state.safety.imuCalibrated ? "IMU READY" : "CALIBRATION")
    : "IMU LOCK";
}

function updateControlDisplay() {
  const throttlePercent = Math.round(state.control.throttle / 10);
  elements.leftStickReadout.textContent = `T ${throttlePercent} / Y ${Math.round(state.control.yaw)}`;
  elements.rightStickReadout.textContent = `P ${Math.round(state.control.pitch)} / R ${Math.round(state.control.roll)}`;
  elements.throttleStrip.textContent = `${throttlePercent}%`;
  elements.throttleMeter.style.width = `${throttlePercent}%`;
  elements.leftStick.style.setProperty("--stick-x", `${50 + (state.control.yaw / 120) * 50}%`);
  elements.leftStick.style.setProperty("--stick-y", `${100 - throttlePercent}%`);
  elements.rightStick.style.setProperty("--stick-x", `${50 + (state.control.roll / 25) * 50}%`);
  elements.rightStick.style.setProperty("--stick-y", `${50 - (state.control.pitch / 25) * 50}%`);
  updateSafetyControls();
}

function resetPilotControls(resetThrottle = true) {
  if (resetThrottle) state.control.throttle = 0;
  state.control.roll = 0;
  state.control.pitch = 0;
  state.control.yaw = 0;
  updateControlDisplay();
}

function buildCommand() {
  const sequence = state.nextSequence++;
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
  return { sequence, packet };
}

function sendCommand(trackLatency = true) {
  if (!state.connected || !state.socket || state.socket.readyState !== WebSocket.OPEN) return false;
  const command = buildCommand();
  try {
    state.socket.send(command.packet);
    if (trackLatency) state.pendingCommands.set(command.sequence, performance.now());
    while (state.pendingCommands.size > 30) {
      state.pendingCommands.delete(state.pendingCommands.keys().next().value);
    }
    return true;
  } catch (error) {
    setAlert("critical", "COMMAND FAILED", error.message || "WebSocket write failed.");
    return false;
  }
}

function startCommandStream() {
  clearInterval(state.commandTimer);
  sendCommand();
  state.commandTimer = setInterval(() => sendCommand(), COMMAND_PERIOD_MS);
}

function stopCommandStream() {
  clearInterval(state.commandTimer);
  state.commandTimer = null;
}

function disconnectFlightLink(reason = "Disconnected by operator") {
  stopCommandStream();
  state.armRequested = false;
  state.armed = false;
  resetPilotControls(true);
  const socket = state.socket;
  state.socket = null;
  if (socket && socket.readyState < WebSocket.CLOSING) socket.close(1000, reason);
  setLinkState(false, reason);
}

function connectFlightLink() {
  if (state.connected || state.connecting) {
    disconnectFlightLink();
    return;
  }
  if (location.protocol === "https:") {
    setAlert("critical", "BROWSER SECURITY BLOCK", "Open http://192.168.4.1 on the NanoKit network. HTTPS pages cannot open the controller's plain WebSocket.");
    return;
  }

  const url = websocketUrl();
  state.connecting = true;
  elements.connectButton.disabled = true;
  elements.connectButton.textContent = "Connecting";
  setEvent(`Opening ${url}`);

  let socket;
  try {
    socket = new WebSocket(url);
  } catch (error) {
    state.connecting = false;
    elements.connectButton.disabled = false;
    setAlert("critical", "LINK ERROR", error.message || "Unable to create WebSocket.");
    return;
  }
  state.socket = socket;

  socket.addEventListener("open", () => {
    if (state.socket !== socket) return;
    state.lastTelemetryAt = Date.now();
    setLinkState(true, `WebSocket active at ${url}`);
    setAlert("warning", "SAFETY GATES ACTIVE", "The controller is connected. Arming remains blocked until IMU, calibration, and ESC protocol checks pass.");
    startCommandStream();
  });

  socket.addEventListener("message", (event) => handleFlightPacket(event.data));
  socket.addEventListener("error", () => {
    setAlert("critical", "LINK ERROR", "The Flight Deck could not maintain the WebSocket connection.");
  });
  socket.addEventListener("close", (event) => {
    if (state.socket !== socket) return;
    state.socket = null;
    stopCommandStream();
    state.armRequested = false;
    state.armed = false;
    resetPilotControls(true);
    setLinkState(false, `WebSocket closed (${event.code})`);
    setAlert("critical", "FLIGHT LINK LOST", "Commands stopped. Firmware failsafe forces all motors to minimum.");
  });
}

function updateLatency(sequence) {
  const sentAt = state.pendingCommands.get(Number(sequence));
  if (sentAt === undefined) return;
  state.latencyMs = Math.max(0, Math.round(performance.now() - sentAt));
  state.pendingCommands.delete(Number(sequence));
  elements.latencyValue.textContent = `${state.latencyMs} ms`;
}

function handleFlightPacket(payload) {
  const fields = parsePacket(payload);
  if (fields.PROTO !== PROTOCOL_VERSION) {
    setAlert("critical", "PROTOCOL MISMATCH", "Upload the current NanoKit Drone 4X Wi-Fi firmware.");
    return;
  }

  if (fields.TYPE === "HELLO") {
    setEvent(`${fields.DEVICE || "NanoKit-Drone-4X"} Wi-Fi link confirmed`);
    return;
  }
  if (fields.TYPE === "ACK") {
    updateLatency(fields.SEQ);
    return;
  }
  if (fields.TYPE === "ERROR") {
    setAlert("critical", fields.CODE || "COMMAND ERROR", fields.MESSAGE || "Firmware rejected the command.");
    return;
  }
  if (fields.TYPE !== "TEL") return;

  state.lastTelemetryAt = Date.now();
  state.telemetry = fields;
  if (fields.ACK) updateLatency(fields.ACK);

  const wasArmed = state.armed;
  state.armed = fields.ARM === "1";
  if (!wasArmed && state.armed) state.armedSince = Date.now();
  if (wasArmed && !state.armed && state.armedSince) {
    state.accumulatedFlightMs += Date.now() - state.armedSince;
    state.armedSince = 0;
    state.armRequested = false;
    resetPilotControls(true);
  }

  state.safety.imuHealthy = fieldEnabled(fields, "IMU");
  state.safety.imuCalibrated = fieldEnabled(fields, "CAL");
  state.safety.attitudeValid = fieldEnabled(fields, "ATT_VALID");
  state.safety.escConfirmed = fieldEnabled(fields, "ESC_OK");
  state.safety.gnssFix = fieldEnabled(fields, "GNSS");
  state.safety.barometerValid = fieldEnabled(fields, "BARO");
  state.safety.environmentValid = fieldEnabled(fields, "ENV");
  state.safety.powerValid = fieldEnabled(fields, "POWER");
  state.safety.navigationReady = fieldEnabled(fields, "NAV");

  const attitudeValid = state.safety.imuHealthy && state.safety.attitudeValid;
  const roll = attitudeValid ? formatNumber(fields.ROLL) : null;
  const pitch = attitudeValid ? formatNumber(fields.PITCH) : null;
  const yaw = attitudeValid ? formatNumber(fields.YAW) : null;
  elements.rollValue.textContent = roll === null ? "--.- deg" : `${roll} deg`;
  elements.pitchValue.textContent = pitch === null ? "--.- deg" : `${pitch} deg`;
  elements.headingValue.textContent = yaw === null ? "--- deg" : `${yaw} deg`;

  const altitude = state.safety.barometerValid ? formatNumber(fields.ALT, 2) : null;
  elements.altitudeValue.textContent = altitude === null ? "--.- m" : `${altitude} m`;
  const speed = state.safety.gnssFix ? formatNumber(fields.SPD, 2) : null;
  elements.speedValue.textContent = speed === null ? "--.- m/s" : `${speed} m/s`;

  const voltage = state.safety.powerValid ? formatNumber(fields.BAT_V, 2) : null;
  const current = state.safety.powerValid ? formatNumber(fields.BAT_A, 2) : null;
  const power = state.safety.powerValid ? formatNumber(fields.BAT_W, 1) : null;
  elements.voltageValue.textContent = voltage === null ? "--.- V" : `${voltage} V`;
  elements.currentValue.textContent = current === null ? "--.- A" : `${current} A`;
  elements.powerValue.textContent = power === null ? "--.- W" : `${power} W`;
  elements.batteryTop.textContent = voltage === null ? "--.- V" : `${voltage} V`;

  elements.gnssTop.textContent = state.safety.gnssFix ? "FIX" : "NO FIX";
  elements.satelliteValue.textContent = state.safety.gnssFix ? (fields.SAT || "0") : "--";
  elements.coordinateValue.textContent = state.safety.gnssFix
    ? `${formatNumber(fields.LAT, 7)}, ${formatNumber(fields.LON, 7)}`
    : "--.-------, --.-------";
  elements.commandAgeValue.textContent = fields.CMD_AGE ? `${fields.CMD_AGE} ms` : "-- ms";
  elements.modeValue.textContent = fields.MODE || state.flightMode;
  elements.flowValue.textContent = fieldEnabled(fields, "FLOW") ? "TRACKING" : "OFFLINE";
  elements.obstacleValue.textContent = fieldEnabled(fields, "OBST") ? "ACTIVE" : "OFFLINE";

  const temperature = state.safety.environmentValid ? formatNumber(fields.TEMP) : null;
  const humidity = state.safety.environmentValid ? formatNumber(fields.HUM) : null;
  elements.temperatureValue.textContent = temperature === null ? "--.- degC" : `${temperature} degC`;
  elements.humidityValue.textContent = humidity === null ? "--%" : `${humidity}%`;

  const telemetryThrottle = state.armed ? Number(fields.THR || 0) : 0;
  if (state.armed && Number.isFinite(telemetryThrottle)) state.control.throttle = clamp(telemetryThrottle, 0, 1000);
  updateControlDisplay();
  updateSafetyControls();
  presentSafetyStatus(fields);
}

function presentSafetyStatus(fields) {
  const status = fields.STATUS || "Safety state updated";
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

function beginArmHold(event) {
  event.preventDefault();
  if (elements.armHoldButton.disabled || state.armHoldTimer) return;
  elements.armHoldButton.classList.add("holding");
  state.armHoldTimer = setTimeout(() => {
    state.armHoldTimer = null;
    elements.armHoldButton.classList.remove("holding");
    state.armRequested = true;
    sendCommand(false);
    setEvent("ARM requested; waiting for firmware safety approval");
  }, ARM_HOLD_MS);
}

function cancelArmHold() {
  clearTimeout(state.armHoldTimer);
  state.armHoldTimer = null;
  elements.armHoldButton.classList.remove("holding");
}

function disarm() {
  cancelArmHold();
  state.armRequested = false;
  resetPilotControls(true);
  sendCommand(false);
  setEvent("DISARM command sent");
}

function emergencyStop() {
  cancelArmHold();
  state.armRequested = false;
  state.stopPulseUntil = Date.now() + 350;
  resetPilotControls(true);
  sendCommand(false);
  setAlert("critical", "EMERGENCY STOP", "Emergency stop sent. Firmware latches the arm inhibit and forces every motor to minimum.");
}

function requestCalibration() {
  if (elements.calibrateImuButton.disabled) return;
  state.calibrationPulseUntil = Date.now() + 350;
  sendCommand(false);
  setEvent("IMU calibration requested; keep the frame level and motionless");
}

function bindStick(zone, side) {
  let pointerId = null;

  const updateFromPointer = (event) => {
    if (!state.connected || event.pointerId !== pointerId) return;
    const rect = zone.getBoundingClientRect();
    const x = clamp((event.clientX - rect.left) / rect.width, 0, 1);
    const y = clamp((event.clientY - rect.top) / rect.height, 0, 1);
    if (side === "left") {
      state.control.yaw = (x * 2 - 1) * 120;
      if (state.armed) state.control.throttle = Math.round((1 - y) * 1000);
    } else {
      state.control.roll = (x * 2 - 1) * 25;
      state.control.pitch = (1 - y * 2) * 25;
    }
    updateControlDisplay();
  };

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

function switchView(view) {
  state.view = view;
  document.querySelectorAll(".mode-button").forEach((button) => {
    button.classList.toggle("active", button.dataset.view === view);
  });
  elements.flightDeck.classList.toggle("fpv-mode", view === "fpv");
  elements.flightDeck.classList.toggle("camera-mode", view === "camera");
  elements.videoStage.hidden = view === "mission";
  elements.missionWorkspace.hidden = view !== "mission";
}

function chooseFlightMode(mode) {
  state.flightMode = mode;
  document.querySelectorAll(".flight-mode").forEach((button) => {
    button.classList.toggle("active", button.dataset.flightMode === mode);
  });
  elements.modeValue.textContent = mode;
  sendCommand(false);
}

function setCameraLive(live, message) {
  state.cameraLive = live;
  elements.cameraFeed.hidden = !live;
  elements.offlineReference.hidden = live;
  setStatusDot(elements.cameraSignalDot, live ? "good" : "warning");
  elements.cameraSignalText.textContent = live ? "CAMERA LIVE" : "CAMERA STANDBY";
  elements.streamMode.textContent = live ? "LIVE CAMERA" : "REFERENCE VIEW";
  elements.frameInfo.textContent = message;
}

function loadCameraStream() {
  if (!state.cameraUrl) {
    setCameraLive(false, "Camera stream URL is not configured");
    return;
  }
  elements.cameraEndpointLabel.textContent = state.cameraUrl;
  elements.cameraFeed.onload = () => setCameraLive(true, "Observation stream online");
  elements.cameraFeed.onerror = () => setCameraLive(false, "Camera endpoint unavailable");
  elements.cameraFeed.src = `${state.cameraUrl}${state.cameraUrl.includes("?") ? "&" : "?"}t=${Date.now()}`;
}

async function refreshPayloadStatus() {
  if (!state.cameraApiUrl) return;
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), 2500);
  try {
    const response = await fetch(`${state.cameraApiUrl.replace(/\/$/, "")}/api/status`, {
      signal: controller.signal,
      cache: "no-store",
    });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const payload = await response.json();
    state.payload.cameraReady = payload.cameraReady === true;
    state.payload.audioCaptureReady = payload.audioCaptureReady === true;
    state.payload.audioPlaybackReady = payload.audioPlaybackReady === true;
    state.payload.storageReady = payload.storageReady === true;
    state.payload.psram8MB = payload.psram8MB === true;
    elements.payloadState.textContent = state.payload.cameraReady ? "READY" : "LOCKED";
  } catch (_error) {
    Object.keys(state.payload).forEach((key) => { state.payload[key] = false; });
    elements.payloadState.textContent = "OFFLINE";
  } finally {
    clearTimeout(timeout);
    updatePayloadControls();
  }
}

function updatePayloadControls() {
  elements.snapshotButton.disabled = !state.payload.cameraReady;
  elements.recordButton.disabled = !(state.payload.cameraReady && state.payload.storageReady);
  elements.audioButton.disabled = !state.payload.audioCaptureReady;
  elements.speakerButton.disabled = !state.payload.audioPlaybackReady;
  elements.cameraTilt.disabled = !state.payload.cameraReady;
}

async function postPayload(path, body) {
  try {
    const response = await fetch(`${state.cameraApiUrl.replace(/\/$/, "")}${path}`, {
      method: "POST",
      headers: { "Content-Type": "text/plain" },
      body: JSON.stringify(body),
    });
    const payload = await response.json().catch(() => ({}));
    if (!response.ok) throw new Error(payload.reason || `HTTP ${response.status}`);
    return payload;
  } catch (error) {
    setAlert("warning", "PAYLOAD COMMAND REJECTED", error.message || "Camera node unavailable.");
    return null;
  }
}

function readStoredMission() {
  try {
    const stored = JSON.parse(localStorage.getItem(MISSION_KEY) || "[]");
    if (Array.isArray(stored)) state.waypoints = stored.slice(0, 50);
  } catch (_error) {
    state.waypoints = [];
  }
}

function renderMission() {
  elements.missionPath.replaceChildren();
  const namespace = "http://www.w3.org/2000/svg";
  if (state.waypoints.length > 1) {
    const path = document.createElementNS(namespace, "polyline");
    path.setAttribute("points", state.waypoints.map((point) => `${point.x * 10},${point.y * 6}`).join(" "));
    path.setAttribute("fill", "none");
    path.setAttribute("stroke", "#72d6ef");
    path.setAttribute("stroke-width", "3");
    path.setAttribute("vector-effect", "non-scaling-stroke");
    elements.missionPath.append(path);
  }
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
  localStorage.setItem(MISSION_KEY, JSON.stringify(state.waypoints));
  updateSafetyControls();
}

function saveMissionFile() {
  const documentBody = {
    format: "nanokit-drone-4x-local-mission",
    version: 1,
    uploaded: false,
    note: "Local planning only until GNSS and navigation feature gates are verified.",
    waypoints: state.waypoints,
  };
  const blob = new Blob([JSON.stringify(documentBody, null, 2)], { type: "application/json" });
  const link = document.createElement("a");
  link.href = URL.createObjectURL(blob);
  link.download = "nanokit-drone-4x-mission.json";
  link.click();
  URL.revokeObjectURL(link.href);
}

function updateClocks() {
  const liveFlightMs = state.armed && state.armedSince ? Date.now() - state.armedSince : 0;
  const totalSeconds = Math.floor((state.accumulatedFlightMs + liveFlightMs) / 1000);
  const minutes = String(Math.floor(totalSeconds / 60)).padStart(2, "0");
  const seconds = String(totalSeconds % 60).padStart(2, "0");
  elements.flightTimeValue.textContent = `${minutes}:${seconds}`;
  if (state.connected && Date.now() - state.lastTelemetryAt > 1200) {
    setStatusDot(elements.linkDot, "warning");
    elements.linkText.textContent = "STALE";
  }
}

function openSettings() {
  elements.flightHost.value = state.flightHost;
  elements.cameraUrl.value = state.cameraUrl;
  elements.cameraApiUrl.value = state.cameraApiUrl;
  elements.settingsDialog.showModal();
}

function saveSettings() {
  const previousHost = normalizeHost(state.flightHost);
  state.flightHost = normalizeHost(elements.flightHost.value);
  state.cameraUrl = elements.cameraUrl.value.trim();
  state.cameraApiUrl = elements.cameraApiUrl.value.trim().replace(/\/$/, "");
  localStorage.setItem(SETTINGS_KEY, JSON.stringify({
    flightHost: state.flightHost,
    cameraUrl: state.cameraUrl,
    cameraApiUrl: state.cameraApiUrl,
  }));
  elements.cameraEndpointLabel.textContent = state.cameraUrl || "No stream endpoint";
  if (state.connected && previousHost !== state.flightHost) disconnectFlightLink("Flight host changed");
  loadCameraStream();
  refreshPayloadStatus();
}

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
elements.fullscreenButton.addEventListener("click", () => {
  if (document.fullscreenElement) document.exitFullscreen();
  else elements.videoStage.requestFullscreen().catch((error) => setEvent(error.message));
});

document.querySelectorAll(".mode-button").forEach((button) => {
  button.addEventListener("click", () => switchView(button.dataset.view));
});
document.querySelectorAll(".flight-mode").forEach((button) => {
  button.addEventListener("click", () => chooseFlightMode(button.dataset.flightMode));
});

bindStick(elements.leftStick, "left");
bindStick(elements.rightStick, "right");

elements.snapshotButton.addEventListener("click", () => postPayload("/api/camera", { action: "snapshot" }));
elements.recordButton.addEventListener("click", async () => {
  const next = !state.recording;
  if (await postPayload("/api/recording", { action: next ? "start" : "stop" })) {
    state.recording = next;
    elements.recordButton.classList.toggle("active", next);
    elements.recordButton.lastChild.textContent = next ? " Stop" : " Record";
  }
});
elements.audioButton.addEventListener("click", async () => {
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
  elements.cameraTiltValue.textContent = `${elements.cameraTilt.value} deg`;
});
elements.cameraTilt.addEventListener("change", () => {
  postPayload("/api/servo", { angle: Number(elements.cameraTilt.value) });
});

elements.missionMap.addEventListener("click", (event) => {
  if (event.target.closest("p") || state.waypoints.length >= 50) return;
  const rect = elements.missionMap.getBoundingClientRect();
  state.waypoints.push({
    x: clamp(((event.clientX - rect.left) / rect.width) * 100, 0, 100),
    y: clamp(((event.clientY - rect.top) / rect.height) * 100, 0, 100),
  });
  renderMission();
});
elements.clearMissionButton.addEventListener("click", () => {
  state.waypoints = [];
  renderMission();
});
elements.saveMissionButton.addEventListener("click", saveMissionFile);
elements.uploadMissionButton.addEventListener("click", () => {
  setAlert("warning", "NAVIGATION LOCKED", "Mission upload stays disabled until GNSS and navigation hardware are verified.");
});

elements.settingsForm.addEventListener("submit", () => saveSettings());
elements.closeSettings.addEventListener("click", () => elements.settingsDialog.close());
elements.cancelSettings.addEventListener("click", () => elements.settingsDialog.close());

document.addEventListener("keydown", (event) => {
  if (event.key === "Escape" && !elements.settingsDialog.open) emergencyStop();
});
document.addEventListener("visibilitychange", () => {
  if (document.hidden && state.connected) disarm();
});
window.addEventListener("beforeunload", () => {
  state.armRequested = false;
  state.control.throttle = 0;
  sendCommand(false);
});

readStoredMission();
renderMission();
elements.cameraEndpointLabel.textContent = state.cameraUrl || "No stream endpoint";
updateControlDisplay();
updatePayloadControls();
switchView("standard");
loadCameraStream();
refreshPayloadStatus();
state.uiTimer = setInterval(updateClocks, 250);
state.payloadTimer = setInterval(refreshPayloadStatus, 5000);
setLinkState(false, "Flight Deck ready; Wi-Fi link not connected");
setAlert("warning", "SYSTEM LOCKED", "Connect to NanoKit-Drone-4X Wi-Fi, open http://192.168.4.1, then connect the WebSocket link.");
