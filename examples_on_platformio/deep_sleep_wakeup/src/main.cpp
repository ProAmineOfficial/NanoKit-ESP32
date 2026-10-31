// Developed by Amine Saoud ibn al-Bashir.
#include <Arduino.h>
#include <esp_sleep.h>   // Deep sleep API: wake-up sources, wake-up cause, and esp_deep_sleep_start().

namespace {
constexpr gpio_num_t WAKE_BUTTON_PIN = GPIO_NUM_33; // NanoKit Pin 20, GPIO33. RTC-capable GPIO: only the RTC domain stays alive in deep sleep, so only RTC GPIOs can wake the chip (ext0).
constexpr uint8_t BUILTIN_LED_PIN = 2;              // NanoKit Pin 17, GPIO2. Onboard LED gives visible feedback at every wake-up, even with the Serial Monitor closed.
constexpr uint32_t SERIAL_BAUD = 115200;            // Standard NanoKit Serial Monitor speed.
constexpr uint64_t SLEEP_SECONDS = 10;              // How long the timer lets the chip sleep before waking it on its own.
constexpr uint64_t MICROSECONDS_PER_SECOND = 1000000ULL; // The sleep API counts microseconds, so seconds must be scaled.
constexpr uint8_t WAKE_BLINKS = 3;                  // Three short blinks mark one wake-up; easy to count by eye.
constexpr uint32_t BLINK_MS = 150;                  // On and off time of one blink.
constexpr uint32_t SERIAL_SETTLE_MS = 100;          // Short pause after the reset so the first Serial lines are not lost by the USB bridge.

RTC_DATA_ATTR uint32_t bootCount = 0;               // Placed in RTC slow memory, which keeps its power during deep sleep; normal RAM variables are lost at every wake-up.

const char *wakeupReason(esp_sleep_wakeup_cause_t cause) {
  switch (cause) {
    case ESP_SLEEP_WAKEUP_TIMER: return "timer (sleep period elapsed)";       // The RTC timer counted down the programmed microseconds.
    case ESP_SLEEP_WAKEUP_EXT0:  return "external button on GPIO33 (ext0)";   // The RTC GPIO saw the level we asked for (HIGH).
    default:                     return "power-on / reset (not a deep-sleep wake-up)"; // First boot, reset button, or a fresh upload.
  }
}

void blinkWakeSignal() {
  for (uint8_t i = 0; i < WAKE_BLINKS; ++i) {       // delay() is acceptable here: the program is sequential and ends in deep sleep, there is no loop to keep responsive.
    digitalWrite(BUILTIN_LED_PIN, HIGH);
    delay(BLINK_MS);
    digitalWrite(BUILTIN_LED_PIN, LOW);
    delay(BLINK_MS);
  }
}
} // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(SERIAL_SETTLE_MS);                            // Give the USB-serial bridge a moment after the deep-sleep reset before printing.
  pinMode(BUILTIN_LED_PIN, OUTPUT);                  // GPIO2 drives the onboard LED.
  pinMode(WAKE_BUTTON_PIN, INPUT);                   // Plain input: the external 10 kOhm resistor already pulls GPIO33 to GND, so no internal pull is needed.

  ++bootCount;                                       // Survives deep sleep because it lives in RTC memory; it resets only on a power cycle or a reset.
  const esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause(); // Ask the chip why it started: timer, ext0, or a plain reset.

  Serial.println("NanoKit deep sleep wake-up example.");
  Serial.print("Boot number: ");
  Serial.println(bootCount);
  Serial.print("Wake-up cause: ");
  Serial.println(wakeupReason(cause));
  Serial.print("Wake button on NanoKit Pin 20 / GPIO33 reads ");
  Serial.println(digitalRead(WAKE_BUTTON_PIN) == HIGH ? "HIGH (pressed, or the pull-down is missing)" : "LOW (released, pull-down OK)");

  Serial.println("Blinking the onboard LED on NanoKit Pin 17 / GPIO2 three times.");
  blinkWakeSignal();

  esp_sleep_enable_timer_wakeup(SLEEP_SECONDS * MICROSECONDS_PER_SECOND); // Wake-up source 1: the RTC timer, 10 s from now.
  esp_sleep_enable_ext0_wakeup(WAKE_BUTTON_PIN, 1);  // Wake-up source 2: wake as soon as GPIO33 is HIGH, i.e. when the button connects it to 3.3 V.

  Serial.print("Timer wake-up armed for ");
  Serial.print(static_cast<uint32_t>(SLEEP_SECONDS));
  Serial.println(" s. Button wake-up armed on NanoKit Pin 20 / GPIO33 (HIGH level).");
  Serial.println("Entering deep sleep now.");
  Serial.flush();                                    // Serial.print only queues bytes; deep sleep cuts the UART power at once, so wait until every byte has left the chip.
  esp_deep_sleep_start();                            // Power down the CPU and RAM. The chip restarts from the boot ROM on wake-up, so nothing after this line ever runs.
}

void loop() {
  // Never reached. Deep sleep does not resume where it left off: every wake-up is a fresh reset that
  // runs setup() again from the top, and setup() ends by going back to sleep. Everything the example
  // needs to remember between wake-ups is kept in RTC memory (bootCount), not in loop() state.
}
