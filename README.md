# IoT-Based Air Quality Monitoring System

An ESP32-based real-time indoor air quality monitoring system that measures temperature, humidity, and air quality (gas/pollutant levels), displays live readings on a 16x2 I2C LCD, streams telemetry to **Blynk** (mobile dashboard + pollution alerts), and logs historical data to **ThingSpeak**.

## Features
- **Real-Time Sensing:** Reads ambient temperature & humidity via **DHT22** and air quality/gas concentration via **MQ135** analog output.
- **Non-Blocking Architecture:** Uses `BlynkTimer` for sensor polling, LCD screen cycling, and cloud uploads—no blocking `delay()` calls in `loop()`.
- **Dual Cloud Integration:**
  - **Blynk:** Live gauges on virtual pins `V0` (Temperature), `V1` (Humidity), `V2` (Gas ADC value), plus debounced `pollution_alert` notifications.
  - **ThingSpeak:** Respects the 15-second API rate limit by logging `field1`, `field2`, and `field3` every 20 seconds.
- **Local Feedback:** Cycles temperature, humidity, and air quality status (`Fresh Air` / `Bad Air`) on a 16x2 I2C LCD (`0x27`) with a custom degree (`°`) symbol.

## Hardware & Wiring

| Component | ESP32 Pin | Notes |
|---|---|---|
| **DHT22** (Data) | `GPIO 4` | 3.3V power + 10kΩ pull-up resistor |
| **MQ135** (Analog Out) | `GPIO 32` | ADC1 channel (safe alongside Wi-Fi) |
| **16x2 I2C LCD** (`0x27`) | `GPIO 21` (SDA), `GPIO 22` (SCL) | Standard ESP32 I2C pins |

## Software Dependencies (Arduino IDE)
- `WiFi` (ESP32 core)
- `Blynk` (`BlynkSimpleEsp32.h`)
- `DHT sensor library` by Adafruit (`DHT.h`)
- `LiquidCrystal_I2C` (`LiquidCrystal_I2C.h`)

## Setup & Configuration
1. Open `iot_air_quality_monitoring.ino` in Arduino IDE (board: **ESP32 Dev Module**).
2. Replace the placeholder credentials at the top of the sketch:
   - `BLYNK_TEMPLATE_ID`, `BLYNK_AUTH_TOKEN`
   - `THINGSPEAK_API_KEY`
   - `WIFI_SSID`, `WIFI_PASS`
3. Upload to the ESP32 and open Serial Monitor at `115200` baud.

## Author
**Mahmoud A. Mohamed** — Mechatronics Engineering, Egypt-Japan University for Science and Technology (E-JUST). ThingSpeak
