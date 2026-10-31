// Developed by Amine Saoud ibn al-Bashir.
// SPDX-License-Identifier: AGPL-3.0-only
#include <Arduino.h>
#include <Wire.h>

namespace {
constexpr uint8_t I2C_SDA_PIN = 21;          // NanoKit Pin 23, GPIO21. The board's dedicated I2C0 data pin (VR_I2C_SDA0); also the Arduino core default SDA.
constexpr uint8_t I2C_SCL_PIN = 22;          // NanoKit Pin 18, GPIO22. The board's dedicated I2C0 clock pin (VR_I2C_SCL0); also the Arduino core default SCL.
constexpr uint32_t SERIAL_BAUD = 115200;     // Standard NanoKit Serial Monitor speed.
constexpr uint32_t I2C_CLOCK_HZ = 100000;    // 100 kHz "standard mode": every I2C device ever made supports it, so the scan cannot miss a slow one.
constexpr uint8_t FIRST_ADDRESS = 0x08;      // 0x00-0x07 are reserved by the I2C specification (general call, start byte, 10-bit prefix, ...).
constexpr uint8_t LAST_ADDRESS = 0x77;       // 0x78-0x7F are reserved too, so a real device never answers above 0x77.
constexpr uint8_t I2C_ERROR_TIMEOUT = 5;     // endTransmission() code when a line stays low and the transfer never completes.
constexpr uint32_t SCAN_INTERVAL_MS = 5000;  // Rescan every 5 s so a module can be plugged in or swapped while the firmware runs.

struct AddressHint {
  uint8_t first;       // Inclusive start of the address range.
  uint8_t last;        // Inclusive end; equal to first for a single address.
  const char *device;  // The chip most often found there. A hint, not a proof: many chips share one address.
};

constexpr AddressHint HINTS[] = {            // Sorted by address so the table doubles as a cheat sheet.
  {0x23, 0x23, "BH1750 light sensor"},
  {0x27, 0x27, "PCF8574 LCD backpack"},
  {0x29, 0x29, "VL53L0X / VL53L1X distance sensor"},
  {0x3C, 0x3D, "SSD1306 OLED display"},
  {0x3F, 0x3F, "PCF8574 LCD backpack"},
  {0x40, 0x40, "INA219 / INA226 current monitor"},
  {0x48, 0x48, "ADS1115 ADC"},
  {0x50, 0x57, "AT24C EEPROM"},
  {0x5A, 0x5A, "MLX90614 IR thermometer"},
  {0x68, 0x69, "MPU-6050 IMU or DS3231 RTC"},
  {0x76, 0x77, "BME280 / BMP280 environment sensor"},
};

uint32_t lastScanMs = 0;                     // Timestamp of the previous scan; the loop stays non-blocking in between.
uint32_t scanCount = 0;                      // Numbered scans make it obvious when a module appears or disappears.

const char *hintFor(uint8_t address) {
  for (const AddressHint &hint : HINTS) {
    if (address >= hint.first && address <= hint.last) {
      return hint.device;
    }
  }
  return "unknown device (check the module datasheet)";
}

void printFound(uint8_t address) {
  Serial.print("  Found device at 0x");
  if (address < 0x10) {
    Serial.print('0');                       // Keep two hex digits so 0x08 does not print as 0x8.
  }
  Serial.print(address, HEX);
  Serial.print("  -> ");
  Serial.println(hintFor(address));
}

void printNoDeviceHelp(uint8_t timeouts) {
  Serial.println("  No device answered. Check, in this order:");
  Serial.println("  1. Power: module VCC on 3V3 (NanoKit Pin 13 or 14), GND on NanoKit Pin 12 or 31.");
  Serial.println("  2. Wires: SDA on NanoKit Pin 23 / GPIO21 and SCL on NanoKit Pin 18 / GPIO22, not swapped.");
  Serial.println("  3. Pull-ups: both lines need a resistor to 3.3 V; add 4.7 kOhm if the module has none.");
  if (timeouts > 0) {
    Serial.println("  Some addresses timed out: a line is probably held low (short, or module not powered)."); // A NACK is instant; a timeout means SDA or SCL never released.
  }
}

void scanBus() {
  uint8_t found = 0;
  uint8_t timeouts = 0;
  for (uint8_t address = FIRST_ADDRESS; address <= LAST_ADDRESS; ++address) {
    Wire.beginTransmission(address);         // Queue a START condition plus the 7-bit address with the write bit, no data.
    const uint8_t error = Wire.endTransmission(); // Send it. 0 = a device pulled SDA low on the 9th clock (ACK); 2 = nobody answered (NACK).
    if (error == 0) {
      printFound(address);
      ++found;
    } else if (error == I2C_ERROR_TIMEOUT) {
      ++timeouts;
    }
  }

  if (found == 0) {
    printNoDeviceHelp(timeouts);
  } else {
    Serial.print("  ");
    Serial.print(found);
    Serial.println(found == 1 ? " device found." : " devices found.");
  }
}
} // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);      // Route the I2C0 peripheral to the NanoKit pins; the core also enables the weak internal pull-ups.
  Wire.setClock(I2C_CLOCK_HZ);               // Slow clock for scanning: works with every device and tolerates long breadboard wires.
  Serial.println("NanoKit I2C scanner ready.");
  Serial.println("Bus: SDA on NanoKit Pin 23 / GPIO21, SCL on NanoKit Pin 18 / GPIO22, 100 kHz.");
  Serial.println("Scanning addresses 0x08-0x77 every 5 s. Plug a module in at any time.");
}

void loop() {
  const uint32_t nowMs = millis();
  const bool firstScan = (scanCount == 0);   // Do not make the user wait 5 s for the first result.
  if (!firstScan && nowMs - lastScanMs < SCAN_INTERVAL_MS) {
    return;                                  // Nothing to do yet; returning keeps the loop responsive for future additions.
  }
  lastScanMs = nowMs;
  ++scanCount;

  Serial.print("Scan ");
  Serial.print(scanCount);
  Serial.println(":");
  scanBus();                                 // About 112 short transactions; at 100 kHz the whole scan takes well under a second.
}
