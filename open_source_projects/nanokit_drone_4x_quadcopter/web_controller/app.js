// NanoKit Drone 4X Flight Deck.
// Developer by Amine Saud Ibn Bashir.
// The camera stream is intentionally separate from BLE flight control.

// These UUIDs must match the NanoKit flight-controller firmware exactly.
const SERVICE_UUID = "e1c91000-7a2a-4fd9-9d62-8ad3525b6f10";
const CONTROL_UUID = "e1c91001-7a2a-4fd9-9d62-8ad3525b6f10";
const TELEMETRY_UUID = "e1c91002-7a2a-4fd9-9d62-8ad3525b6f10";
const DEFAULT_CAMERA_URL = "http://nanokit-camera.local/stream";
const CAMERA_STORAGE_KEY = "nanokit-drone-4x-camera-url";

// State is kept together so every outbound command packet is predictable.
const state = {
  device: null,
  controlCharacteristic: null,
  connected: false,
  armed: false,
  calibrated: false,
  throttle: 0,
  roll: 0,
  pitch: 0,
  yaw: 0,
  sendTimer: null,
  commandWriteInProgress: false,
  commandWritePromise: Promise.resolve(),
  nextCommandSequence: 1,
  lastAcknowledgedSequence: 0,
  cameraUrl: localStorage.getItem(CAMERA_STORAGE_KEY) || DEFAULT_CAMERA_URL,
  cameraLive: false,
};

const encoder = new TextEncoder();

// Cache all DOM elements once; handlers below then stay concise and readable.
const elements = {
  connectButton: document.querySelector("#connectButton"),
  disconnectButton: document.querySelector("#disconnectButton"),
  armButton: document.querySelector("#armButton"),
  stopButton: document.querySelector("#stopButton"),
  calibrateButton: document.querySelector("#calibrateButton"),
  holdButton: document.querySelector("#holdButton"),
  throttleSlider: document.querySelector("#throttleSlider"),
  throttleDisplay: document.querySelector("#throttleDisplay"),
  throttleValue: document.querySelector("#throttleValue"),
  throttleLevel: document.querySelector("#throttleLevel"),
  throttleUp: document.querySelector("#throttleUp"),
  throttleDown: document.querySelector("#throttleDown"),
  throttleZero: document.querySelector("#throttleZero"),
  connectionStatus: document.querySelector("#connectionStatus"),
  linkState: document.querySelector("#linkState"),
  linkIndicator: document.querySelector("#linkIndicator"),
  deviceName: document.querySelector("#deviceName"),
  messageLine: document.querySelector("#messageLine"),
  rollValue: document.querySelector("#rollValue"),
  pitchValue: document.querySelector("#pitchValue"),
  yawValue: document.querySelector("#yawValue"),
  calibrationState: document.querySelector("#calibrationState"),
  systemNote: document.querySelector(".system-note"),
  cameraFrame: document.querySelector("#cameraFrame"),
  cameraPreview: document.querySelector("#cameraPreview"),
  cameraFallback: document.querySelector("#cameraFallback"),
  cameraStatus: document.querySelector("#cameraStatus"),
  cameraLiveIndicator: document.querySelector("#cameraLiveIndicator"),
  cameraResolution: document.querySelector("#cameraResolution"),
  cameraClock: document.querySelector("#cameraClock"),
  loadCameraButton: document.querySelector("#loadCameraButton"),
  fullscreenCameraButton: document.querySelector("#fullscreenCameraButton"),
  settingsButton: document.querySelector("#settingsButton"),
  cameraDialog: document.querySelector("#cameraDialog"),
  cameraForm: document.querySelector("#cameraForm"),
  cameraUrl: document.querySelector("#cameraUrl"),
  closeCameraSettings: document.querySelector("#closeCameraSettings"),
  cancelCameraSettings: document.querySelector("#cancelCameraSettings"),
};

// Display one short, high-priority status message below the connection strip.
function setMessage(message) {
  elements.messageLine.textContent = message;
}

// Keep connection text, LEDs, and protected controls coherent with the BLE state.
function setConnected(connected) {
  state.connected = connected;
  elements.connectionStatus.textContent = connected ? "Connected" : "Disconnected";
  elements.linkState.textContent = connected ? "GATT connected; waiting for telemetry" : "Standby";
  elements.connectButton.disabled = connected;
  elements.disconnectButton.disabled = !connected;
  elements.linkIndicator.classList.toggle("connected", connected);
  updateFlightControlAvailability();
}

// Keep throttle locked until the firmware confirms both calibration and arming.
// This mirrors the board's independent zero-throttle safety rule in the UI.
function updateFlightControlAvailability() {
  const canArm = state.connected && (state.armed || state.calibrated);
  const canThrottle = state.connected && state.armed;
  elements.armButton.disabled = !canArm;
  elements.calibrateButton.disabled = !state.connected;
  elements.throttleSlider.disabled = !canThrottle;
  elements.throttleUp.disabled = !canThrottle;
  elements.throttleDown.disabled = !canThrottle;
  elements.throttleZero.disabled = !state.connected;
}

// Clamp and render throttle before it is ever included in an outbound BLE packet.
function updateThrottle(value) {
  state.throttle = Math.max(0, Math.min(1000, Number(value)));
  const percentage = (state.throttle / 1000) * 100;
  elements.throttleSlider.value = String(state.throttle);
  elements.throttleDisplay.textContent = String(state.throttle);
  elements.throttleValue.textContent = String(state.throttle);
  elements.throttleLevel.style.setProperty("--level", `${percentage}%`);
}

// Mirror the arm state in the visual control without trusting the UI as the only safeguard.
function updateArmButton() {
  elements.armButton.textContent = state.armed ? "Disarm System" : "Arm System";
  elements.armButton.classList.toggle("armed", state.armed);
  updateFlightControlAvailability();
}

// Construct the semicolon-separated protocol documented in Bluetooth_Protocol.md.
function buildPacket(extra = "") {
  const sequence = state.nextCommandSequence++;
  return `T=${state.throttle};R=${state.roll};P=${state.pitch};Y=${state.yaw};A=${state.armed ? 1 : 0};SEQ=${sequence};${extra}`;
}

// Prefer an acknowledged write because it interoperates reliably with the
// ESP32 Arduino BLE stack and confirms that the GATT command reached the drone.
async function writeControlPacket(packet) {
  const characteristic = state.controlCharacteristic;
  if (characteristic?.properties?.write && typeof characteristic.writeValueWithResponse === "function") {
    await characteristic.writeValueWithResponse(packet);
    return;
  }

  if (characteristic?.properties?.writeWithoutResponse &&
      typeof characteristic.writeValueWithoutResponse === "function") {
    await characteristic.writeValueWithoutResponse(packet);
    return;
  }

  throw new Error("Drone control characteristic does not support a compatible GATT write.");
}

// Serialize GATT writes. A new heartbeat is skipped while the previous packet
// is in flight, preventing concurrent-write failures in Chrome on Windows.
async function sendPacket(extra = "", waitForTurn = false) {
  if (!state.connected || !state.controlCharacteristic) {
    return false;
  }

  while (state.commandWriteInProgress) {
    if (!waitForTurn) {
      return false;
    }
    await state.commandWritePromise.catch(() => undefined);
  }

  const packet = encoder.encode(buildPacket(extra));
  state.commandWriteInProgress = true;
  state.commandWritePromise = writeControlPacket(packet);
  try {
    await state.commandWritePromise;
    return true;
  } finally {
    state.commandWriteInProgress = false;
  }
}

// A 100 ms heartbeat is comfortably within the firmware's 700 ms failsafe window.
function startCommandStream() {
  clearInterval(state.sendTimer);
  state.sendTimer = setInterval(() => {
    sendPacket().catch((error) => {
      stopCommandStream();
      setMessage(describeCommandWriteError(error));
    });
  }, 100);
}

// Stop heartbeat traffic as soon as the GATT session ends.
function stopCommandStream() {
  clearInterval(state.sendTimer);
  state.sendTimer = null;
}

// Decode the compact firmware telemetry packet into key/value fields.
function parseTelemetry(payload) {
  const fields = {};
  payload.split(";").forEach((part) => {
    const [key, value] = part.split("=");
    if (key && value !== undefined) {
      fields[key.trim()] = value.trim();
    }
  });
  return fields;
}

// Do not present an absent sensor as a level, motionless aircraft. Dashes make
// the wiring or firmware state unambiguous until valid attitude data exists.
function showImuUnavailable() {
  elements.rollValue.textContent = "--";
  elements.pitchValue.textContent = "--";
  elements.yawValue.textContent = "--";
}

// Update instruments from the latest NanoKit telemetry notification.
function handleTelemetry(event) {
  const payload = new TextDecoder().decode(event.target.value);
  const fields = parseTelemetry(payload);

  if (fields.PROTO !== "2") {
    state.calibrated = false;
    updateFlightControlAvailability();
    showImuUnavailable();
    elements.linkState.textContent = "Telemetry received from legacy firmware";
    setMessage("Legacy firmware detected. Upload the current Drone 4X firmware over USB.");
    return;
  }

  if (fields.IMU !== "1") {
    state.calibrated = false;
    updateFlightControlAvailability();
    showImuUnavailable();
    elements.systemNote.classList.remove("calibrated");
    elements.calibrationState.textContent = "MPU6050 not detected";
    elements.linkState.textContent = "BLE active; IMU hardware unavailable";
    setMessage("MPU6050 not detected. Check 3.3 V, GND, SDA GPIO21, SCL GPIO22, and address 0x68/0x69.");
    return;
  }

  if (fields.ROLL) elements.rollValue.textContent = `${fields.ROLL} deg`;
  if (fields.PITCH) elements.pitchValue.textContent = `${fields.PITCH} deg`;
  if (fields.YAW) elements.yawValue.textContent = `${fields.YAW} deg`;
  if (fields.THR) elements.throttleValue.textContent = fields.THR;
  if (fields.ARM) {
    state.armed = fields.ARM === "1";
    updateArmButton();
  }

  if (fields.ACK !== undefined) {
    state.lastAcknowledgedSequence = Number(fields.ACK);
  }

  const calibrated = fields.CAL === "1";
  state.calibrated = calibrated;
  updateFlightControlAvailability();
  elements.systemNote.classList.toggle("calibrated", calibrated);
  elements.calibrationState.textContent = calibrated ? "IMU calibration valid" : "IMU calibration required";
  elements.linkState.textContent = fields.ACK !== undefined
    ? `BLE active; command ${state.lastAcknowledgedSequence} acknowledged`
    : "BLE telemetry active; waiting for command acknowledgement";
  if (!calibrated) {
    setMessage("IMU calibration is required before arming.");
  } else {
    setMessage(fields.LINK === "1" ? "Telemetry live." : "Firmware command timeout.");
  }
}

// Update the clock overlay without placing timing work inside the flight-command heartbeat.
function updateCameraClock() {
  elements.cameraClock.textContent = new Date().toLocaleTimeString([], { hour12: false });
}

// Render a valid live camera state once the image or MJPEG endpoint loads.
function setCameraLive(live, status) {
  state.cameraLive = live;
  elements.cameraPreview.hidden = !live;
  elements.cameraFallback.hidden = live;
  elements.cameraLiveIndicator.textContent = live ? "LIVE" : "OFFLINE";
  elements.cameraLiveIndicator.classList.toggle("live", live);
  elements.cameraResolution.textContent = live ? "STREAM ONLINE" : "NO SIGNAL";
  elements.cameraStatus.textContent = status;
}

// Load the configured camera URL in the central viewport; it never affects flight controls.
function loadCameraStream() {
  const url = state.cameraUrl.trim();
  if (!url) {
    setCameraLive(false, "Camera endpoint is empty");
    return;
  }
  setCameraLive(false, "Connecting to camera node...");
  elements.cameraPreview.src = url;
}

// A malformed or unreachable endpoint returns to the safe offline viewport.
elements.cameraPreview.addEventListener("error", () => {
  setCameraLive(false, "Camera endpoint unavailable");
});

// The first image of an MJPEG stream marks the camera as live.
elements.cameraPreview.addEventListener("load", () => {
  setCameraLive(true, "Camera stream connected");
});

// Open the browser BLE device picker, then subscribe to the telemetry characteristic.
async function connect() {
  if (!navigator.bluetooth) {
    setMessage("Web Bluetooth is not available in this browser.");
    return;
  }

  const device = await navigator.bluetooth.requestDevice({
    filters: [{ namePrefix: "NanoKit-Drone-4X" }],
    optionalServices: [SERVICE_UUID],
  });

  device.addEventListener("gattserverdisconnected", () => {
    state.controlCharacteristic = null;
    state.device = null;
    state.armed = false;
    state.calibrated = false;
    updateArmButton();
    setConnected(false);
    stopCommandStream();
    setMessage("Bluetooth disconnected. Firmware has disarmed.");
  });

  const server = await device.gatt.connect();
  const service = await server.getPrimaryService(SERVICE_UUID);
  state.controlCharacteristic = await service.getCharacteristic(CONTROL_UUID);
  const telemetryCharacteristic = await service.getCharacteristic(TELEMETRY_UUID);
  telemetryCharacteristic.addEventListener("characteristicvaluechanged", handleTelemetry);
  await telemetryCharacteristic.startNotifications();

  state.device = device;
  elements.deviceName.textContent = device.name || "NanoKit-Drone-4X";
  setConnected(true);
  startCommandStream();
  setMessage("Connected. Keep propellers removed for all bench tests.");
}

// Turn browser-specific BLE failures into instructions the operator can act on.
function describeBleError(error) {
  if (error?.name === "NotFoundError") {
    return "No drone was selected. Click Connect BLE, select NanoKit-Drone-4X, then choose Associate.";
  }

  if (error?.name === "SecurityError") {
    return "Bluetooth permission was blocked. Allow Bluetooth for this browser, then try Connect BLE again.";
  }

  if (error?.name === "NetworkError") {
    return "The BLE link was interrupted. Restart the drone, then reconnect from this page.";
  }

  return error?.message || "Bluetooth connection failed. Check the Serial Monitor and retry.";
}

// Explain the write stage separately, because discovery and GATT service access
// may already have succeeded when a command characteristic rejects a packet.
function describeCommandWriteError(error) {
  if (error?.name === "NetworkError") {
    return "BLE command link dropped. Firmware will disarm; restart the drone and reconnect.";
  }

  if (error?.name === "NotSupportedError") {
    return "The connected drone firmware has an incompatible control characteristic. Upload the current firmware over USB, then reconnect.";
  }

  return `BLE command write failed${error?.name ? ` (${error.name})` : ""}. Firmware will disarm safely.`;
}

// Stop the vehicle command before deliberately closing the Bluetooth connection.
async function disconnect() {
  await emergencyStop();
  if (state.device?.gatt?.connected) {
    state.device.gatt.disconnect();
  }
}

// Reset all flight axes, throttle, and arm state before sending the emergency command.
async function emergencyStop() {
  state.armed = false;
  state.roll = 0;
  state.pitch = 0;
  state.yaw = 0;
  updateThrottle(0);
  updateArmButton();
  await sendPacket("", true);
  setMessage("Emergency stop sent.");
}

// Direction buttons are momentary so releasing a control always recentres that axis.
function bindMomentaryButton(button) {
  const roll = Number(button.dataset.roll || 0);
  const pitch = Number(button.dataset.pitch || 0);
  const yaw = Number(button.dataset.yaw || 0);

  const press = () => {
    state.roll = roll;
    state.pitch = pitch;
    state.yaw = yaw;
    button.classList.add("active");
  };
  const release = () => {
    state.roll = 0;
    state.pitch = 0;
    state.yaw = 0;
    button.classList.remove("active");
  };

  button.addEventListener("pointerdown", press);
  button.addEventListener("pointerup", release);
  button.addEventListener("pointerleave", release);
  button.addEventListener("pointercancel", release);
}

// Bind every pitch, roll, and yaw control to the same safe momentary behaviour.
document.querySelectorAll("[data-roll], [data-pitch], [data-yaw]").forEach(bindMomentaryButton);

// Start BLE discovery only after the operator selects the explicit connect command.
elements.connectButton.addEventListener("click", () => {
  connect().catch((error) => setMessage(describeBleError(error)));
});

// Send a software stop before closing the selected GATT device.
elements.disconnectButton.addEventListener("click", () => {
  disconnect().catch((error) => setMessage(error.message));
});

// Require zero UI throttle before requesting arm; firmware enforces the same independently.
elements.armButton.addEventListener("click", async () => {
  if (!state.armed && state.throttle !== 0) {
    setMessage("Set throttle to zero before arming.");
    return;
  }
  if (!state.armed && !state.calibrated) {
    setMessage("Calibrate the IMU on a level, motionless frame before arming.");
    return;
  }
  state.armed = !state.armed;
  updateArmButton();
  await sendPacket("", true);
});

// Keep the emergency stop command available even if the BLE session is already gone.
elements.stopButton.addEventListener("click", () => {
  emergencyStop().catch((error) => setMessage(error.message));
});

// Calibration requests always start from a disarmed, zero-throttle state.
elements.calibrateButton.addEventListener("click", async () => {
  await emergencyStop();
  await sendPacket("CAL=1;", true);
  setMessage("IMU calibration requested. Keep the frame still and level.");
});

// Centre attitude and yaw commands while preserving the deliberate throttle setting.
elements.holdButton.addEventListener("click", () => {
  state.roll = 0;
  state.pitch = 0;
  state.yaw = 0;
  setMessage("Pitch, roll, and yaw commands centred.");
});

// All throttle controls share the same clamped update function.
elements.throttleSlider.addEventListener("input", (event) => updateThrottle(event.target.value));
elements.throttleUp.addEventListener("click", () => updateThrottle(state.throttle + 50));
elements.throttleDown.addEventListener("click", () => updateThrottle(state.throttle - 50));
elements.throttleZero.addEventListener("click", () => updateThrottle(0));

// Camera controls only load or display the independent Wi-Fi stream.
elements.loadCameraButton.addEventListener("click", loadCameraStream);
elements.fullscreenCameraButton.addEventListener("click", () => {
  if (document.fullscreenElement) {
    document.exitFullscreen().catch(() => setMessage("Could not exit camera fullscreen."));
  } else {
    elements.cameraFrame.requestFullscreen().catch(() => setMessage("Camera fullscreen is not available."));
  }
});

// The gear button edits a local camera URL without transmitting it to the flight controller.
elements.settingsButton.addEventListener("click", () => {
  elements.cameraUrl.value = state.cameraUrl;
  elements.cameraDialog.showModal();
});

// Both close actions leave the saved endpoint unchanged.
elements.closeCameraSettings.addEventListener("click", () => elements.cameraDialog.close());
elements.cancelCameraSettings.addEventListener("click", () => elements.cameraDialog.close());

// Store the valid camera endpoint locally, then load it in the central viewport.
elements.cameraForm.addEventListener("submit", (event) => {
  event.preventDefault();
  const nextUrl = elements.cameraUrl.value.trim();
  if (!nextUrl) {
    setMessage("Enter a camera stream URL first.");
    return;
  }
  try {
    new URL(nextUrl);
  } catch {
    setMessage("Camera URL is not valid.");
    return;
  }
  state.cameraUrl = nextUrl;
  localStorage.setItem(CAMERA_STORAGE_KEY, state.cameraUrl);
  elements.cameraDialog.close();
  loadCameraStream();
});

// Initialise the cockpit in its safest state and keep the camera clock current.
updateThrottle(0);
updateArmButton();
setConnected(false);
setCameraLive(false, "Awaiting camera endpoint");
updateCameraClock();
setInterval(updateCameraClock, 1000);
