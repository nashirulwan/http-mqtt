# Laporan Projek: Komparasi Protokol MQTT vs HTTP pada Sistem IoT

## 1. Skema Rangkaian

Komponen:

- ESP32 DevKit
- Sensor infrared obstacle
- Kabel jumper
- Laptop sebagai server lokal

Koneksi rangkaian:

```text
IR Sensor VCC  -> 3V3 ESP32
IR Sensor GND  -> GND ESP32
IR Sensor OUT  -> GPIO 4 ESP32
```

Masukkan foto/diagram rangkaian di bagian ini.

## 2. Kode Program

Kode yang digunakan:

- Firmware ESP32: `firmware/esp32_ir_http_mqtt/esp32_ir_http_mqtt.ino`
- HTTP Server Flask: `server/http_server.py`
- MQTT Subscriber Python: `server/mqtt_subscriber.py`
- MQTT Broker config: `server/mosquitto.conf`

Penjelasan singkat:

Pada skenario HTTP, ESP32 bertindak sebagai HTTP client. ESP32 membaca kondisi sensor infrared, membuat data JSON, lalu mengirim data dengan metode POST ke endpoint `http://IP-LAPTOP:8000/sensor`. Server Flask menerima data, memberi respon `200 OK`, dan menyimpan data ke file `data/http_sensor_data.jsonl`.

Pada skenario MQTT, ESP32 bertindak sebagai publisher. ESP32 membaca kondisi sensor infrared, membuat data JSON, lalu publish ke topic `iot/sensor/esp32-ir-01`. Laptop menjalankan Mosquitto sebagai broker dan script Python sebagai subscriber. Data yang diterima disimpan ke file `data/mqtt_sensor_data.jsonl`.

Contoh struktur JSON:

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

## 3. Analisis Perbandingan

MQTT menggunakan pola publish/subscribe. ESP32 hanya mengirim pesan ke topic, kemudian subscriber yang membutuhkan data dapat menerima pesan dari broker. Pola ini cocok untuk IoT karena ringan dan fleksibel untuk data sensor berkala.

HTTP menggunakan pola request/response. ESP32 mengirim data ke endpoint server dengan metode POST, lalu server memberikan respon. HTTP lebih mudah dipahami dan dites karena konsep endpoint REST API umum digunakan, tetapi setiap pengiriman data memerlukan request dan response.

Dari sisi kemudahan coding, HTTP lebih sederhana karena cukup membuat endpoint `/sensor` dan mengirim JSON dengan POST. MQTT sedikit lebih kompleks karena membutuhkan broker, topic, publisher, dan subscriber.

Dari sisi kecepatan, isi bagian ini setelah pengujian:

```text
Hasil pengamatan Serial Monitor:
- HTTP: sekitar ... ms
- MQTT: sekitar ... ms

Kesimpulan: protokol yang lebih cepat pada pengujian ini adalah ... karena ...
```

Kedua protokol menggunakan JSON agar data sensor mudah dibaca dan mudah diproses oleh server.

## 4. Bukti Screenshot

Masukkan screenshot berikut:

- Foto/diagram rangkaian ESP32 dan sensor infrared.
- Serial Monitor ESP32 saat mengirim data via MQTT.
- Serial Monitor ESP32 saat mengirim data via HTTP.
- Tampilan log MQTT subscriber atau MQTT Explorer.
- Tampilan log HTTP server Flask.
- Tampilan isi file `data/http_sensor_data.jsonl`.
- Tampilan isi file `data/mqtt_sensor_data.jsonl`.

Contoh perintah untuk menampilkan log:

```bash
tail -f data/http_sensor_data.jsonl
tail -f data/mqtt_sensor_data.jsonl
```

## 5. Demo dan Presentasi

Langkah demo:

1. Jalankan server laptop dengan `./scripts/start-all.sh`.
2. Upload firmware ke ESP32.
3. Buka Serial Monitor pada baud rate `115200`.
4. Dekatkan objek ke sensor infrared.
5. Tunjukkan data masuk ke log HTTP dan MQTT.
6. Jelaskan perbedaan MQTT dan HTTP berdasarkan hasil pengamatan.
