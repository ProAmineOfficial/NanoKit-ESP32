# Project Backlog

**Developed by Amine Saoud ibn al-Bashir.**

The complete list of examples, applications, and tutorials planned for NanoKit Integrated ESP32, grouped in waves. Every entry is implemented from scratch for NanoKit (original code and documentation, NanoKit pin numbers, `nanokit.yaml` manifest, generated diagrams, CI build). Simple, interface-free projects go to `examples_on_platformio/`; projects with a web interface (buttons, sliders, dashboards served from the board) go to `applications_on_platformio/`.

The **Status** column is maintained by `python tools/update_repository_index.py`: `done` when the folder exists and carries `nanokit.yaml` (or, for tutorials and articles, when the file exists), `in progress` when the folder exists without a manifest, `planned` otherwise. Edit every other column by hand.

Rules for new entries: pick pins from `registry/nanokit-esp32.yaml`; analog inputs on ADC1 (GPIO32-39) so Wi-Fi never breaks them; I2C on I2C0 (Pin 23 / GPIO21, Pin 18 / GPIO22); UART2 on Pin 39 / Pin 40; VSPI on Pin 24 / Pin 16 / Pin 38 with CS on Pin 36; never GPIO6-11, never outputs on GPIO34-39; 5 V signals always behind a level shifter.

## Wave 1 - Board fundamentals (no extra hardware beyond LED, button, potentiometer)

<!-- BACKLOG:TABLE -->
| Topic | Area | Folder | Wave | Hardware | Status |
|---|---|---|---:|---|---|
| Digital input and output: button and LED | example | examples_on_platformio/digital_input_output | 1 | Button, LED, 220 ohm | done |
| PWM: LED fade with LEDC | example | examples_on_platformio/pwm_led_fade | 1 | LED, 220 ohm | done |
| Analog input: potentiometer on ADC1 | example | examples_on_platformio/analog_input_potentiometer | 1 | 10 kOhm potentiometer | done |
| GPIO interrupts and hardware timers | example | examples_on_platformio/interrupts_and_timers | 1 | Button, LED, 220 ohm | done |
| Deep sleep: timer and button wake-up, RTC memory | example | examples_on_platformio/deep_sleep_wakeup | 1 | Button, 10 kOhm | done |
| Capacitive touch pads | example | examples_on_platformio/touch_sensor | 1 | Jumper wire, LED | done |
| Built-in Hall-effect sensor | example | examples_on_platformio/hall_sensor | 1 | Magnet | done |
| I2C scanner | example | examples_on_platformio/i2c_scanner | 1 | Any I2C module | done |
| Dual-core FreeRTOS tasks | example | examples_on_platformio/dual_core_tasks | 1 | LED, 220 ohm | done |
| Flash storage with Preferences (NVS) | example | examples_on_platformio/preferences_flash_storage | 1 | Button | done |

## Wave 2 - Sensors, displays, and modules

| Topic | Area | Folder | Wave | Hardware | Status |
|---|---|---|---:|---|---|
| DHT11 / DHT22 temperature and humidity | example | examples_on_platformio/dht_temperature_humidity | 2 | DHT11 or DHT22 | planned |
| BME280 temperature, humidity, pressure (I2C) | example | examples_on_platformio/bme280_environment | 2 | BME280 module | planned |
| BME680 gas, temperature, humidity, pressure | example | examples_on_platformio/bme680_air_quality | 2 | BME680 module | planned |
| DS18B20 one-wire temperature | example | examples_on_platformio/ds18b20_temperature | 2 | DS18B20, 4.7 kOhm | planned |
| Multiple DS18B20 on one bus | example | examples_on_platformio/ds18b20_multiple_sensors | 2 | 2+ DS18B20, 4.7 kOhm | planned |
| BMP180 barometric pressure | example | examples_on_platformio/bmp180_pressure | 2 | BMP180 module | planned |
| BMP388 precision pressure and altitude | example | examples_on_platformio/bmp388_altitude | 2 | BMP388 module | planned |
| MPU-6050 accelerometer and gyroscope | example | examples_on_platformio/mpu6050_motion | 2 | MPU-6050 module | planned |
| PIR motion detector | example | examples_on_platformio/pir_motion_detector | 2 | HC-SR501 PIR | planned |
| SSD1306 OLED display (I2C) | example | examples_on_platformio/oled_ssd1306_display | 2 | 0.96" OLED | planned |
| 16x2 LCD with I2C backpack | example | examples_on_platformio/lcd_i2c_display | 2 | LCD1602 + PCF8574 | planned |
| OLED temperature readout | example | examples_on_platformio/oled_temperature_readout | 2 | OLED + BME280 | planned |
| Relay module control | example | examples_on_platformio/relay_module_control | 2 | 3.3 V relay module | planned |
| DC motor with L298N driver | example | examples_on_platformio/dc_motor_l298n | 2 | L298N, DC motor | planned |
| Servo motor control | example | examples_on_platformio/servo_motor_control | 2 | SG90 servo | planned |
| MicroSD card read and write (SPI) | example | examples_on_platformio/microsd_card_files | 2 | microSD module | planned |
| MicroSD data logging with timestamps | example | examples_on_platformio/microsd_data_logging | 2 | microSD module, DS18B20 | planned |
| TCA9548A I2C multiplexer | example | examples_on_platformio/i2c_multiplexer_tca9548a | 2 | TCA9548A, 2 OLEDs | planned |
| Stepper motor control (28BYJ-48) | example | examples_on_platformio/stepper_motor_control | 2 | ULN2003, 28BYJ-48 | done |
| Ultrasonic distance (HC-SR04) | example | examples_on_platformio/ultrasonic_distance | 2 | HC-SR04 | done |
| Camera pan/tilt tracking from Serial | example | examples_on_platformio/camera_tracking | 2 | 2 servos | done |

## Wave 3 - Web applications (interface served from the board, LittleFS `data/` folder)

| Topic | Area | Folder | Wave | Hardware | Status |
|---|---|---|---:|---|---|
| Output control web app (buttons toggle GPIOs) | application | applications_on_platformio/web_output_control | 3 | LEDs | planned |
| PWM slider web app | application | applications_on_platformio/web_pwm_slider | 3 | LED | planned |
| Multiple PWM sliders web app | application | applications_on_platformio/web_pwm_multi_slider | 3 | 3 LEDs or RGB LED | planned |
| Relay control web app | application | applications_on_platformio/web_relay_control | 3 | Relay module | planned |
| Servo control web app | application | applications_on_platformio/web_servo_control | 3 | SG90 servo | planned |
| DHT dashboard web app | application | applications_on_platformio/web_dht_dashboard | 3 | DHT22 | planned |
| BME280 dashboard web app | application | applications_on_platformio/web_bme280_dashboard | 3 | BME280 | planned |
| BME680 dashboard web app | application | applications_on_platformio/web_bme680_dashboard | 3 | BME680 | planned |
| DS18B20 dashboard web app | application | applications_on_platformio/web_ds18b20_dashboard | 3 | DS18B20 | planned |
| Live chart web app (single series) | application | applications_on_platformio/web_live_chart | 3 | BME280 | planned |
| Live chart web app (multiple series) | application | applications_on_platformio/web_live_chart_multi | 3 | BME280 | planned |
| Thermostat web app with setpoint | application | applications_on_platformio/web_thermostat | 3 | DS18B20, relay | planned |
| Momentary switch web app (press-and-hold) | application | applications_on_platformio/web_momentary_switch | 3 | LED | planned |
| Physical button synchronised with web app | application | applications_on_platformio/web_physical_button_sync | 3 | Button, LED | planned |
| Input fields web app (save settings) | application | applications_on_platformio/web_input_fields | 3 | none | planned |
| Images and files from LittleFS web app | application | applications_on_platformio/web_littlefs_files | 3 | none | planned |
| RGB LED colour picker web app | application | applications_on_platformio/web_rgb_color_picker | 3 | RGB LED | planned |
| Timer / pulse output web app | application | applications_on_platformio/web_timer_pulse | 3 | LED or relay | planned |
| HTTP authentication web app | application | applications_on_platformio/web_http_auth | 3 | LED | planned |
| MPU-6050 3D orientation web app | application | applications_on_platformio/web_mpu6050_orientation | 3 | MPU-6050 | planned |
| MicroSD file browser web app | application | applications_on_platformio/web_microsd_browser | 3 | microSD module | planned |
| Stepper motor web app (HTTP) | application | applications_on_platformio/web_stepper_control | 3 | ULN2003, 28BYJ-48 | planned |
| Stepper motor web app (WebSocket) | application | applications_on_platformio/web_stepper_websocket | 3 | ULN2003, 28BYJ-48 | planned |
| Gauges dashboard web app | application | applications_on_platformio/web_gauges_dashboard | 3 | BME280 | planned |
| Async web server with live updates (SSE) | application | applications_on_platformio/web_async_sse | 3 | BME280 | planned |
| Web server with WebSocket control panel | application | applications_on_platformio/web_websocket_panel | 3 | LEDs | planned |
| Face rotation tracking camera | application | applications_on_platformio/face_rotation_tracking_camera | 3 | 2 servos | done |

## Wave 4 - Protocols, cloud, and messaging

| Topic | Area | Folder | Wave | Hardware | Status |
|---|---|---|---:|---|---|
| Wi-Fi station: connect, IP, RSSI, reconnect | example | examples_on_platformio/wifi_station_basics | 4 | none | planned |
| Wi-Fi access point mode | example | examples_on_platformio/wifi_access_point | 4 | none | planned |
| Fixed IP address and hostname | example | examples_on_platformio/wifi_fixed_ip_hostname | 4 | none | planned |
| HTTP GET and POST client | example | examples_on_platformio/http_client_get_post | 4 | none | planned |
| MQTT publish and subscribe | example | examples_on_platformio/mqtt_publish_subscribe | 4 | none | planned |
| MQTT sensor publisher (DHT) | example | examples_on_platformio/mqtt_dht_publisher | 4 | DHT22 | planned |
| ESP-NOW one-to-one | example | examples_on_platformio/espnow_one_to_one | 4 | 2 NanoKit boards | planned |
| ESP-NOW two-way | example | examples_on_platformio/espnow_two_way | 4 | 2 NanoKit boards | planned |
| ESP-NOW one-to-many and many-to-one | example | examples_on_platformio/espnow_many_boards | 4 | 3 NanoKit boards | planned |
| BLE server and client | example | examples_on_platformio/ble_server_client | 4 | 2 NanoKit boards or phone | planned |
| Bluetooth Classic serial | example | examples_on_platformio/bluetooth_classic_serial | 4 | phone | planned |
| Email alert over SMTP | example | examples_on_platformio/email_alert_smtp | 4 | none | planned |
| NTP time, date, and time zones | example | examples_on_platformio/ntp_time_and_timezones | 4 | none | planned |
| OTA firmware update | example | examples_on_platformio/ota_update | 4 | none | planned |
| Wi-Fi manager (configure credentials from a web page) | application | applications_on_platformio/web_wifi_manager | 4 | none | planned |
| ESP-NOW receiver with web dashboard | application | applications_on_platformio/web_espnow_dashboard | 4 | 2 NanoKit boards | planned |
| MQTT dashboard (Node-RED) | application | applications_on_platformio/mqtt_node_red_dashboard | 4 | broker, DHT22 | planned |
| Firebase realtime database | application | applications_on_platformio/firebase_realtime_database | 4 | Firebase account | planned |
| Firebase web app with sensor readings | application | applications_on_platformio/firebase_web_app | 4 | Firebase account, BME280 | planned |
| Firebase data logging | application | applications_on_platformio/firebase_data_logging | 4 | Firebase account | planned |
| Telegram control outputs | application | applications_on_platformio/telegram_control_outputs | 4 | Telegram bot | planned |
| Telegram sensor readings | application | applications_on_platformio/telegram_sensor_readings | 4 | Telegram bot, BME280 | planned |
| Telegram motion detection | application | applications_on_platformio/telegram_motion_alert | 4 | Telegram bot, PIR | planned |
| ThingSpeak cloud logging | application | applications_on_platformio/thingspeak_logging | 4 | ThingSpeak account | planned |
| Google Sheets logging | application | applications_on_platformio/google_sheets_logging | 4 | Google account | planned |
| MySQL / PHP cloud logging | application | applications_on_platformio/mysql_php_logging | 4 | Web hosting | planned |
| Door monitor with reed switch and notification | application | applications_on_platformio/door_monitor | 4 | Reed switch | planned |
| Solar-powered sensor node with deep sleep | application | applications_on_platformio/solar_sensor_node | 4 | Solar panel, charger, BME280 | planned |
| Weather station (complete) | application | applications_on_platformio/weather_station | 4 | BME280, rain/wind optional | in progress |
| Smart home controller | application | applications_on_platformio/smart_home_controller | 4 | Relays, sensors | in progress |
| Industrial monitoring | application | applications_on_platformio/industrial_monitoring | 4 | Current sensor, relays | in progress |

## Wave 5 - External modules (proposed, need the module)

| Topic | Area | Folder | Wave | Hardware | Status |
|---|---|---|---:|---|---|
| LoRa point-to-point with SX1278 | example | examples_on_platformio/lora_sx1278_link | 5 | 2 LoRa modules | planned |
| SIM800L SMS and GPRS | example | examples_on_platformio/sim800l_sms_gprs | 5 | SIM800L, SIM, 4 V supply | planned |
| OV2640 camera module (SPI or parallel) | example | examples_on_platformio/camera_module_capture | 5 | Camera module | planned |
| ESP-MESH network | example | examples_on_platformio/esp_mesh_network | 5 | 3 NanoKit boards | planned |
| Status PCB / carrier board | application | applications_on_platformio/status_carrier_board | 5 | Custom PCB | planned |
<!-- BACKLOG:TABLE:END -->

## Tutorials and articles (not hardware projects)

| Topic | Area | Path | Status |
|---|---|---|---|
| NanoKit Integrated ESP32 introduction | tutorial | tutorials/beginner/nanokit-introduction.md | planned |
| VS Code and PlatformIO setup | tutorial | tutorials/beginner/vscode-platformio-setup.md | planned |
| Pinout walkthrough | tutorial | tutorials/beginner/pinout-walkthrough.md | planned |
| Troubleshooting uploads and Serial | tutorial | tutorials/beginner/troubleshooting-upload-serial.md | planned |
| LittleFS upload from PlatformIO | tutorial | tutorials/intermediate/littlefs-upload.md | planned |
| Saving data with Preferences | tutorial | tutorials/intermediate/preferences-library.md | planned |
| Reconnect to Wi-Fi and useful Wi-Fi functions | tutorial | tutorials/intermediate/wifi-functions.md | planned |
| OTA updates from PlatformIO | tutorial | tutorials/advanced/ota-platformio.md | planned |
| MAC address, hostname, fixed IP | tutorial | tutorials/intermediate/network-identity.md | planned |
| Understanding GPIO on NanoKit | article | articles/nanokit/understanding-gpio.md | planned |
| Understanding PWM | article | articles/embedded_systems/understanding-pwm.md | planned |
| Understanding ADC | article | articles/embedded_systems/understanding-adc.md | planned |
| Understanding I2C, SPI, UART | article | articles/embedded_systems/understanding-buses.md | planned |
| Low-power design with deep sleep | article | articles/iot/low-power-design.md | planned |
| Real-time tasks on a dual-core MCU | article | articles/embedded_systems/real-time-dual-core.md | planned |
