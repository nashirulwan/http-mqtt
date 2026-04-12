# Komparasi MQTT vs HTTP pada Sistem IoT

Proyek ini berisi contoh lengkap untuk tugas komparasi protokol MQTT dan HTTP pada sistem IoT dengan ESP32 dan sensor infrared.

## Arsitektur

```text
ESP32 + Sensor Infrared
  |-- HTTP POST JSON --> Laptop Flask Server --> data/http_sensor_data.jsonl
  |
  `-- MQTT Publish JSON -> Mosquitto Broker -> Python Subscriber -> data/mqtt_sensor_data.jsonl
```

## Struktur Repo

```text
firmware/esp32_ir_http_mqtt/  Kode ESP32 untuk sensor infrared
server/                       HTTP server, MQTT subscriber, config Mosquitto
scripts/                      Script start, stop, dan test lokal
report/                       Template laporan
requirements.txt              Dependency Python
```

## Rangkaian Sensor Infrared

Contoh untuk modul infrared obstacle sensor 3 pin:

```text
IR Sensor VCC  -> 3V3 ESP32
IR Sensor GND  -> GND ESP32
IR Sensor OUT  -> GPIO 4 ESP32
```

Jika modul sensor infrared memakai output aktif LOW, kode bawaan sudah menganggap objek terdeteksi saat pin `OUT` bernilai `LOW`.

## Jalankan Server Laptop

Install dependency Python:

```bash
cd /home/nashiru/http-mqtt
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

Jika memakai Nix/NixOS, cukup jalankan:

```bash
nix --extra-experimental-features nix-command --extra-experimental-features flakes develop
```

Jalankan semua service:

```bash
./scripts/start-all.sh
```

Tes lokal dari laptop:

```bash
./scripts/test-local.sh
```

Stop service:

```bash
./scripts/stop-all.sh
```

## Konfigurasi ESP32

Edit file `firmware/esp32_ir_http_mqtt/esp32_ir_http_mqtt.ino`:

```cpp
const char* WIFI_SSID = "NamaWiFi";
const char* WIFI_PASSWORD = "PasswordWiFi";
const char* HTTP_SERVER_URL = "http://IP-LAPTOP:8000/sensor";
const char* MQTT_BROKER_HOST = "IP-LAPTOP";
```

Cari IP laptop:

```bash
ip addr
```

Gunakan IP yang satu jaringan dengan ESP32. Jangan gunakan `127.0.0.1` di ESP32 karena itu berarti alamat ESP32 sendiri, bukan laptop.

## Data JSON

HTTP dan MQTT memakai format JSON yang sama:

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

## Bukti Laporan

Gunakan template di `report/LAPORAN_TEMPLATE.md`. Screenshot yang perlu dikumpulkan:

- Foto/diagram rangkaian ESP32 dengan sensor infrared.
- Serial Monitor ESP32 saat mengirim via HTTP dan MQTT.
- Log HTTP server dari `data/http_sensor_data.jsonl`.
- Log MQTT subscriber dari `data/mqtt_sensor_data.jsonl`.
- Terminal saat `./scripts/test-local.sh` berhasil.
