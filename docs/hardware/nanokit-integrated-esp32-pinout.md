# NanoKit Integrated ESP32 Pinout Reference

This document is the repository reference for selecting pins in NanoKit-ESP32 PlatformIO examples and applications.

Use normal ESP32 GPIO numbers in code. Always document the NanoKit board pin number beside the GPIO number in README files, wiring guides, and comments.

## Core Rules

- PlatformIO code uses ESP32 GPIO numbers, not board pin numbers.
- Every example that uses hardware wiring must document `NanoKit Pin -> GPIO -> Component Pin -> Description`.
- GPIO34, GPIO35, GPIO36, and GPIO39 are input-only on ESP32. Do not use them for LEDs, relays, motors, or any output signal.
- GPIO6, GPIO7, GPIO8, GPIO9, GPIO10, and GPIO11 are connected to internal SPI flash on this board. Do not use them in applications.
- GPIO0 is the BOOT pin. Do not drive it low during reset or power-on.
- GPIO1 and GPIO3 are UART0 pins shared with USB serial. Avoid using them when Serial Monitor is needed.
- EN is Reset / Enable. It is not a GPIO.
- GPIO12 has ESP32 boot-strapping considerations. Use it carefully and avoid external circuits that force an unsafe boot level.

## Quick Pin Table

| NanoKit Pin | GPIO | Function | Notes |
|---:|---|---|---|
| 2 | GPIO34 | ADC1_CH6 | Input only |
| 3 | GPIO25 | DAC1 | ADC2_CH8 capable |
| 4 | GPIO39 | SensorVN / ADC1_CH3 | Input only |
| 5 | GPIO36 | SensorVP / ADC1_CH0 | Input only |
| 6 | GPIO35 | ADC1_CH7 | Input only |
| 7 | GPIO32 | Touch9 / ADC1_CH4 | General GPIO capable |
| 8 | GPIO13 | HSPI MOSI | SPI |
| 9 | GPIO15 | HSPI CS | SPI |
| 10 | GPIO4 | Touch0 | General GPIO capable |
| 15 | GPIO14 | HSPI CLK | SPI |
| 16 | GPIO19 | VSPI MISO | SPI |
| 17 | GPIO2 | Built-in LED / PWM capable | General GPIO capable |
| 18 | GPIO22 | I2C SCL | I2C |
| 19 | GPIO27 | Touch7 | General GPIO capable |
| 20 | GPIO33 | ADC1_CH5 | General GPIO capable |
| 23 | GPIO21 | I2C SDA | I2C |
| 24 | GPIO23 | VSPI MOSI | SPI |
| 25 | GPIO1 | UART0 TX | Shared with USB Serial |
| 26 | GPIO3 | UART0 RX | Shared with USB Serial |
| 33 | GPIO12 | HSPI MISO | Boot-strapping considerations |
| 34 | GPIO0 | BOOT | Avoid for normal applications |
| 35 | EN | Reset / Enable | Not a GPIO |
| 36 | GPIO5 | VSPI CS | SPI |
| 37 | GPIO26 | DAC2 | General GPIO capable |
| 38 | GPIO18 | VSPI CLK | SPI |
| 39 | GPIO17 | UART2 TX | UART |
| 40 | GPIO16 | UART2 RX | UART |

## Full 40-Pin Connector Map

### Left Side - Pins 1 to 20

| Pin | Board Address | Board Label | ESP32 GPIO | Notes |
|---:|---|---|---|---|
| 1 | - | Pull-down | - | Board pull-down reference, not a GPIO |
| 2 | 0x5E | VR_ADC_IN6 | GPIO34 | ADC1_CH6, input only |
| 3 | 0x71 | VR_DAC_OUT1 | GPIO25 | DAC1, ADC2_CH8 capable |
| 4 | 0x5B | VR_ADC_IN3 | GPIO39 | SensorVN, ADC1_CH3, input only |
| 5 | 0x58 | VR_ADC_IN0 | GPIO36 | SensorVP, ADC1_CH0, input only |
| 6 | 0x80 | VR_GPIO00 | GPIO35 | ADC1_CH7, input only |
| 7 | 0x5C | VR_ADC_IN4 | GPIO32 | Touch9, ADC1_CH4, general GPIO capable |
| 8 | 0x10 | VR_SPI_MOSI0 | GPIO13 | HSPI MOSI |
| 9 | 0x13 | VR_SPI_CS0 | GPIO15 | HSPI CS |
| 10 | 0x81 | VR_GPIO01 | GPIO4 | Touch0, ADC2_CH0, general GPIO capable |
| 11 | XX | +5V_IN/OUT | - | Power rail |
| 12 | XX | VR_GND | GND | Ground rail |
| 13 | XX | +3V3_OUT | - | 3.3 V output rail |
| 14 | XX | +3V3_OUT | - | 3.3 V output rail |
| 15 | 0x12 | VR_SPI_SCLK0 | GPIO14 | HSPI CLK |
| 16 | 0x15 | VR_SPI_MISO1 | GPIO19 | VSPI MISO |
| 17 | 0xC0 | VR_PWM_OUT0 | GPIO2 | PWM capable, onboard LED according to board design |
| 18 | 0x31 | VR_I2C_SCL0 | GPIO22 | I2C SCL |
| 19 | 0x82 | VR_GPIO02 | GPIO27 | Touch7, ADC2_CH7, general GPIO capable |
| 20 | 0x83 | VR_GPIO03 | GPIO33 | ADC1_CH5, Touch8, general GPIO capable |

### Right Side - Pins 21 to 40

| Pin | Board Address | Board Label | ESP32 GPIO | Notes |
|---:|---|---|---|---|
| 21 | 0x84 | VR_GPIO04 | GPIO11 | Internal SPI flash, do not use |
| 22 | 0x85 | VR_GPIO05 | GPIO10 | Internal SPI flash, do not use |
| 23 | 0x30 | VR_I2C_SDA0 | GPIO21 | I2C SDA |
| 24 | 0x14 | VR_SPI_MOSI1 | GPIO23 | VSPI MOSI |
| 25 | 0x01 | VR_UART_TX0 | GPIO1 | UART0 TX, shared with USB Serial |
| 26 | 0x00 | VR_UART_RX0 | GPIO3 | UART0 RX, shared with USB Serial |
| 27 | 0x86 | VR_GPIO06 | GPIO9 | Internal SPI flash, do not use |
| 28 | 0x87 | VR_GPIO07 | GPIO6 | Internal SPI flash, do not use |
| 29 | 0x88 | VR_GPIO08 | GPIO7 | Internal SPI flash, do not use |
| 30 | 0x89 | VR_GPIO09 | GPIO8 | Internal SPI flash, do not use |
| 31 | XX | VR_GND | GND | Ground rail |
| 32 | XX | +5V_IN/OUT | - | Power rail |
| 33 | 0x11 | VR_SPI_MISO0 | GPIO12 | HSPI MISO, boot-strapping considerations |
| 34 | 0x38 | VR_BOOT | GPIO0 | BOOT pin, must not be forced low at power-on |
| 35 | 0xFF | VR_RST | EN | Reset / Enable, not a GPIO |
| 36 | 0x17 | VR_SPI_CS1 | GPIO5 | VSPI CS |
| 37 | 0x8A | VR_GPIO10 | GPIO26 | DAC2, ADC2_CH9, general GPIO capable |
| 38 | 0x16 | VR_SPI_SCLK1 | GPIO18 | VSPI CLK |
| 39 | 0x05 | VR_UART_TX2 | GPIO17 | UART2 TX |
| 40 | 0x04 | VR_UART_RX2 | GPIO16 | UART2 RX |

## Recommended First Choices

| Use Case | Recommended GPIO | NanoKit Pin | Reason |
|---|---:|---:|---|
| Onboard LED / first digital output | GPIO2 | 17 | Available GPIO and PWM capable |
| I2C SDA | GPIO21 | 23 | Standard ESP32 I2C SDA choice |
| I2C SCL | GPIO22 | 18 | Standard ESP32 I2C SCL choice |
| UART2 TX | GPIO17 | 39 | Keeps USB Serial free |
| UART2 RX | GPIO16 | 40 | Keeps USB Serial free |
| VSPI MOSI | GPIO23 | 24 | Standard ESP32 VSPI MOSI |
| VSPI MISO | GPIO19 | 16 | Standard ESP32 VSPI MISO |
| VSPI CLK | GPIO18 | 38 | Standard ESP32 VSPI clock |
| VSPI CS | GPIO5 | 36 | Standard ESP32 VSPI chip select |

## Wiring Documentation Template

Use this table in every hardware example README or `Wiring.md` file.

| NanoKit Pin | GPIO | Component Pin | Description |
|---:|---|---|---|
| 23 | GPIO21 | OLED SDA | I2C data line |
| 18 | GPIO22 | OLED SCL | I2C clock line |
| 12 or 31 | GND | OLED GND | Common ground |
| 13 or 14 | 3V3 | OLED VCC | 3.3 V power |

## PlatformIO Code Example

```cpp
#include <Arduino.h>

const int LED_PIN = 2;  // NanoKit Pin 17, GPIO2, onboard LED / PWM-capable GPIO.

void setup() {
  pinMode(LED_PIN, OUTPUT);  // Configure GPIO2 as an output so it can drive the onboard LED.
}

void loop() {
  digitalWrite(LED_PIN, HIGH);  // Drive NanoKit Pin 17 high.
  delay(500);                   // Keep the LED on for half a second.
  digitalWrite(LED_PIN, LOW);   // Drive NanoKit Pin 17 low.
  delay(500);                   // Keep the LED off for half a second.
}
```

## Do Not Use in Applications

| NanoKit Pin | GPIO | Reason |
|---:|---|---|
| 21 | GPIO11 | Internal SPI flash |
| 22 | GPIO10 | Internal SPI flash |
| 27 | GPIO9 | Internal SPI flash |
| 28 | GPIO6 | Internal SPI flash |
| 29 | GPIO7 | Internal SPI flash |
| 30 | GPIO8 | Internal SPI flash |

Using these pins can prevent the ESP32 from booting or corrupt normal flash access.
