# Triggerino

**Triggerino** is a modular, ESP32-based trigger interface for professional AV and lighting systems. It supports a wide range of input types and can trigger actions over multiple protocols like **Art-Net**, **OSC**, **HTTP**, and **TCP**.

## Features

- Supports dry contact, analog, PWM, and sensor inputs (GPIO only for now)
- Outputs over Art-Net, OSC, HTTP, and TCP
- Ethernet (LAN8720) & Wi-Fi (with fallback AP setup)
- Built-in web interface for configuration
- Flexible JSON-based logic mappings
- OLED display with live status
- Neopixel-based feedback and animations
- FreeRTOS-powered task separation
- Modular codebase (PlatformIO / ESP-IDF structure)

## Folder Structure

```
src/
├── main.cpp                # RTOS setup and main loop
├── hardware/               # Button, Sensor, Display, LED managers
├── network/                # Network stack and protocol handlers
├── core/                   # Trigger logic and config mappings
├── utils/                  # Shared data types and helpers
data/                       # Web UI and config.json (LittleFS)
```

## Setup Instructions

1. **Clone the repo**:
   ```bash
   git clone https://github.com/ashleyvansteenacker/Triggerino.git
   cd Triggerino
   ```

2. **Install dependencies**:
   This project uses [PlatformIO](https://platformio.org/).
   ```bash
   pio run
   ```

3. **Upload the firmware**:
   ```bash
   pio run -t upload
   ```

4. **Upload web files to LittleFS**:
   ```bash
   pio run -t uploadfs
   ```

5. **Connect via serial** to monitor output:
   ```bash
   pio device monitor
   ```

## Web Interface

When first powered up:
- If no Wi-Fi is configured, it starts in **AP mode** (`Triggerino-Setup`)
- Visit `http://192.168.4.1` to configure Wi-Fi and settings

## Configuration

Stored in `/data/config.json` and parsed at runtime. It supports:
- Inputs (buttons, (sensors not yet))
- Outputs (artnet, http, etc.)
- Mappings with conditions
- Sensor thresholds
- Output modes (live, toggle, static)

## Example
Trigegring Scene 2 of a LSS device

```json
{
  "buttons": [
    {
      "pin": 5,
      "name": "Scene_1",
      "pullup": true,
      "inverted": false
    }
  ],
  "outputs": [
    {
      "type": "http",
      "name": "lss_anim_2",
      "host": "192.168.1.147",
      "port": 80,
      "path": "/SetAutoplay.html?anim=2"
    }
  ],
  "mappings": [
    {
      "input_name": "Scene_1",
      "input_type": "button",
      "condition": "pressed",
      "output_name": "lss_anim_2"
    }
  ]
}
```

## Current state
As of now only the GPIO input works. 
The sensors and other parts will be fixed as we go 


## Contributing

Pull requests are welcome! Please keep changes modular and well-documented.


## Acknowledgements

This project wouldn't be possible without the hard work of the open source community. Special thanks to the authors and maintainers of the following libraries:

- [ArduinoJson](https://arduinojson.org/) by Benoît Blanchon — for efficient JSON parsing and generation.
- [Adafruit BusIO](https://github.com/adafruit/Adafruit_BusIO) and related drivers
- [Adafruit SI1145 Library](https://github.com/adafruit/Adafruit_SI1145_Library) — for UV/light sensor support.
- [Adafruit BME280 Library](https://github.com/adafruit/Adafruit_BME280_Library) — for environmental sensing.
- [Adafruit SSD1306 + GFX](https://github.com/adafruit/Adafruit_SSD1306) — for OLED display support.
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32) — for robust ESP32 support.
- [WiFiUdp / HTTPClient](https://github.com/espressif/arduino-esp32) — part of ESP32 networking libraries.
- [CNMAT OSC](https://github.com/CNMAT/OSC) — for Open Sound Control (OSC) support.
- [PlatformIO](https://platformio.org/) — for a modern embedded development workflow.

Thank you to everyone contributing to these libraries!