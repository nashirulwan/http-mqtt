# MQTT vs HTTP on IoT

A small hands-on project that compares MQTT and HTTP for sending sensor data from an ESP32 to a laptop. The ESP32 reads an infrared obstacle sensor and ships the same JSON payload over both protocols so you can see how they behave side by side.

## Architecture

```text
ESP32 + Infrared Sensor
  |-- HTTP POST JSON --> Flask Server on laptop --> data/http_sensor_data.jsonl
  |
  `-- MQTT Publish JSON -> Mosquitto Broker -> Python subscriber -> data/mqtt_sensor_data.jsonl
```

## Repo Structure

```text
firmware/esp32_ir_http_mqtt/  ESP32 code for the infrared sensor
server/                       HTTP server, MQTT subscriber, Mosquitto config
scripts/                      start, stop, and local test scripts
report/                       report template and write-up
requirements.txt              Python dependencies
flake.nix                     Nix dev shell with everything you need
```

## Hardware Setup

This uses a 3-pin infrared obstacle sensor module:

```text
IR Sensor VCC  -> 3V3 ESP32
IR Sensor GND  -> GND ESP32
IR Sensor OUT  -> GPIO 4 ESP32
```

A lot of these modules are active LOW, meaning `OUT` goes LOW when something is in front of the sensor. The firmware already treats LOW as "object detected", so you don't need to flip anything.

## Run Locally

You need Python and Mosquitto. Pick whichever setup fits your machine.

### With plain Python

```bash
git clone https://github.com/nashirulwan/http-mqtt.git
cd http-mqtt
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

### With Nix

If you're on Nix or NixOS, the flake gives you Python, Mosquitto, PlatformIO, and the rest in one shell:

```bash
nix --extra-experimental-features nix-command --extra-experimental-features flakes develop
```

### Start, test, stop

Bring up the HTTP server, Mosquitto broker, and MQTT subscriber:

```bash
./scripts/start-all.sh
```

Fire a couple of test payloads from the laptop without touching the ESP32:

```bash
./scripts/test-local.sh
```

Shut everything down:

```bash
./scripts/stop-all.sh
```

## Configure the ESP32

Copy the example config and edit it with your WiFi and the laptop's IP:

```bash
cp firmware/esp32_ir_http_mqtt/src/config.example.h firmware/esp32_ir_http_mqtt/src/config.h
```

Then edit `firmware/esp32_ir_http_mqtt/src/config.h`:

```cpp
const char* WIFI_SSID = "YourWiFi";
const char* WIFI_PASSWORD = "YourWiFiPassword";
const char* HTTP_SERVER_URL = "http://LAPTOP-IP:8000/sensor";
const char* MQTT_BROKER_HOST = "LAPTOP-IP";
```

`config.h` is gitignored so your WiFi credentials never end up on GitHub.

Find the laptop's IP with:

```bash
ip addr
```

Use the IP that's on the same network as the ESP32. Don't use `127.0.0.1` on the ESP32, since that points back at the ESP32 itself, not your laptop.

## JSON Payload

HTTP and MQTT send the exact same JSON shape:

```json
{
  "device_id": "esp32-ir-01",
  "sensor": "infrared",
  "ir_detected": true,
  "ir_value": 0,
  "detection_count": 3,
  "sequence": 10,
  "protocol": "mqtt"
}
```

## Report

There's a ready-to-use write-up in `report/LAPORAN.md`, with a blank template at `report/LAPORAN_TEMPLATE.md`.

Things worth capturing if you're documenting a run:

- A photo or diagram of the ESP32 wiring with the infrared sensor.
- The ESP32 serial monitor while it sends over HTTP and MQTT.
- The HTTP server log at `data/http_sensor_data.jsonl`.
- The MQTT subscriber log at `data/mqtt_sensor_data.jsonl`.
- The terminal output of a successful `./scripts/test-local.sh`.

## License

MIT. See [LICENSE](LICENSE).
