# Laporan Projek: Komparasi Protokol MQTT vs HTTP pada Sistem IoT

## 1. Skema Rangkaian

Projek ini menggunakan ESP32 sebagai mikrokontroler dan sensor infrared sebagai input sensor. Laptop digunakan sebagai server lokal untuk menerima data dari ESP32 melalui dua metode komunikasi, yaitu HTTP dan MQTT.

Komponen yang digunakan:

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

Sensor infrared pada program ini dibaca sebagai sensor digital. Jika sensor mendeteksi objek, nilai pin berubah dan ESP32 menaikkan nilai `detection_count`. Foto atau diagram rangkaian dimasukkan pada bagian ini saat penyusunan laporan akhir.

## 2. Kode Program

Kode yang digunakan dalam projek:

- Firmware ESP32: `firmware/esp32_ir_http_mqtt/src/esp32_ir_http_mqtt.cpp`
- Contoh konfigurasi ESP32: `firmware/esp32_ir_http_mqtt/src/config.example.h`
- HTTP Server Flask: `server/http_server.py`
- MQTT Subscriber Python: `server/mqtt_subscriber.py`
- MQTT Broker config: `server/mosquitto.conf`

Pada skenario HTTP, ESP32 bertindak sebagai HTTP client. ESP32 membaca kondisi sensor infrared, membuat data dalam format JSON, lalu mengirimkan data menggunakan metode POST ke endpoint:

```text
http://IP-LAPTOP:8000/sensor
```

Server Flask pada laptop menerima request tersebut, memberi respon `200 OK`, dan menyimpan data ke file:

```text
data/http_sensor_data.jsonl
```

Pada skenario MQTT, ESP32 bertindak sebagai publisher. ESP32 membaca kondisi sensor infrared, membuat data dalam format JSON, lalu mengirimkan data ke broker Mosquitto dengan topic:

```text
iot/sensor/esp32-ir-01
```

Laptop menjalankan Mosquitto sebagai MQTT broker dan menjalankan script Python sebagai subscriber. Data MQTT yang diterima disimpan ke file:

```text
data/mqtt_sensor_data.jsonl
```

Contoh struktur data JSON yang dikirim:

```json
{
  "device_id": "esp32-ir-01",
  "sensor": "infrared",
  "ir_detected": true,
  "ir_value": 0,
  "detection_count": 3,
  "sequence": 10,
  "protocol": "mqtt",
  "wifi_rssi": -55,
  "uptime_ms": 5000
}
```

## 3. Analisis Perbandingan

### Mana yang lebih cepat sampai ke server?

Berdasarkan struktur program yang dibuat, MQTT cenderung lebih cepat dan lebih ringan untuk pengiriman data sensor IoT. Pada kode ESP32, pengiriman MQTT dilakukan dengan `mqttClient.publish(MQTT_TOPIC, payload.c_str())`. ESP32 hanya mengirim pesan ke broker Mosquitto pada topic tertentu.

Pada HTTP, ESP32 mengirim data dengan `http.POST(payload)` ke server Flask. Setelah itu ESP32 juga membaca respon server menggunakan `http.getString()`. Artinya HTTP memakai pola request-response, sehingga prosesnya lebih panjang dibandingkan MQTT publish.

Program ESP32 sudah mencetak waktu pengiriman ke Serial Monitor atau terminal dengan satuan milidetik. Contoh output yang akan muncul:

```text
MQTT publish OK ... ms -> {"device_id":"esp32-ir-01",...}
HTTP POST 200 ... ms -> {"status":"ok",...}
```

Kesimpulan analisis:

```text
MQTT lebih cepat secara konsep dan lebih cocok untuk data sensor berkala karena overhead lebih kecil. HTTP tetap berhasil mengirim data, tetapi memiliki overhead lebih besar karena memakai request dan response.
```

Jika pada saat demo hasil angka berbeda karena kondisi jaringan, kesimpulan angka mengikuti nilai aktual yang muncul di Serial Monitor atau terminal.

### Mana yang lebih mudah dikoding?

HTTP lebih mudah dikoding karena alurnya sederhana. ESP32 cukup mengirim JSON menggunakan metode POST ke endpoint `/sensor`, kemudian server Flask menerima data tersebut dan memberi respon `200 OK`.

MQTT sedikit lebih kompleks karena membutuhkan beberapa bagian tambahan, yaitu broker Mosquitto, topic, publisher, dan subscriber. ESP32 harus publish ke topic tertentu, sedangkan laptop harus menjalankan broker dan subscriber agar data dapat diterima dan dicatat.

Kesimpulan:

```text
HTTP lebih mudah dikoding dan dipahami, sedangkan MQTT lebih cocok untuk sistem IoT karena lebih ringan dan menggunakan pola publish/subscribe.
```

### Penggunaan struktur data JSON

Kedua protokol menggunakan struktur data JSON yang sama agar hasil pengiriman HTTP dan MQTT dapat dibandingkan dengan adil. JSON dipakai karena formatnya mudah dibaca, mudah dikirim lewat jaringan, dan mudah diproses oleh server.

Field yang digunakan:

- `device_id`: identitas ESP32.
- `sensor`: jenis sensor, yaitu `infrared`.
- `ir_detected`: status apakah sensor infrared mendeteksi objek.
- `ir_value`: nilai digital dari pin sensor infrared.
- `detection_count`: jumlah deteksi objek oleh sensor.
- `sequence`: nomor urut pengiriman data.
- `protocol`: metode pengiriman data, yaitu `http` atau `mqtt`.
- `wifi_rssi`: kekuatan sinyal WiFi ESP32.
- `uptime_ms`: lama waktu ESP32 menyala dalam milidetik.

Contoh data HTTP yang berhasil diterima server laptop dari ESP32:

```json
{
  "device_id": "esp32-ir-01",
  "sensor": "infrared",
  "ir_detected": false,
  "ir_value": 1,
  "detection_count": 0,
  "sequence": 44,
  "protocol": "http",
  "wifi_rssi": -65,
  "uptime_ms": 265305
}
```

Contoh data MQTT yang berhasil diterima subscriber laptop dari ESP32:

```json
{
  "device_id": "esp32-ir-01",
  "sensor": "infrared",
  "ir_detected": false,
  "ir_value": 1,
  "detection_count": 0,
  "sequence": 45,
  "protocol": "mqtt",
  "wifi_rssi": -73,
  "uptime_ms": 270305
}
```

## 4. Bukti Screenshot

Serial Monitor adalah tampilan output dari ESP32 saat terhubung ke laptop melalui USB. Jika menggunakan Arduino IDE, fitur ini bernama Serial Monitor. Jika menggunakan PlatformIO atau terminal Linux, screenshot terminal juga bisa digunakan sebagai pengganti Serial Monitor karena isinya tetap berasal dari output serial ESP32.

Contoh perintah terminal untuk membuka Serial Monitor ESP32:

```bash
pio device monitor -b 115200
```

Jika tidak menggunakan PlatformIO, bisa memakai:

```bash
screen /dev/ttyUSB0 115200
```

Port bisa berbeda, misalnya `/dev/ttyACM0`, tergantung board ESP32.

Screenshot yang perlu dimasukkan ke laporan:

- Foto atau diagram rangkaian ESP32 dengan sensor infrared.
- Tampilan Serial Monitor atau terminal ESP32 saat WiFi berhasil terhubung.
- Tampilan Serial Monitor atau terminal ESP32 saat `MQTT publish OK`.
- Tampilan Serial Monitor atau terminal ESP32 saat `HTTP POST 200`.
- Tampilan log HTTP server Flask.
- Tampilan log MQTT subscriber atau MQTT Explorer.
- Tampilan isi file `data/http_sensor_data.jsonl`.
- Tampilan isi file `data/mqtt_sensor_data.jsonl`.

Perintah untuk menampilkan log pada laptop:

```bash
tail -f data/http_sensor_data.jsonl
tail -f data/mqtt_sensor_data.jsonl
```

Contoh bukti dari pengujian ESP32 ke laptop:

```text
HTTP log:
{"received_at": "2026-04-12T13:42:07.769864+00:00", "protocol": "http", "remote_addr": "10.106.114.97", "data": {"device_id": "esp32-ir-01", "sensor": "infrared", "ir_detected": false, "ir_value": 1, "detection_count": 0, "sequence": 44, "protocol": "http", "wifi_rssi": -65, "uptime_ms": 265305}}

MQTT log:
{"received_at": "2026-04-12T13:42:12.818219+00:00", "protocol": "mqtt", "topic": "iot/sensor/esp32-ir-01", "qos": 0, "retain": false, "data": {"device_id": "esp32-ir-01", "sensor": "infrared", "ir_detected": false, "ir_value": 1, "detection_count": 0, "sequence": 45, "protocol": "mqtt", "wifi_rssi": -73, "uptime_ms": 270305}}
```

## 5. Demo dan Presentasi

Langkah demo:

1. Jalankan server laptop dengan `./scripts/start-all.sh`.
2. Upload firmware ke ESP32.
3. Buka Serial Monitor atau terminal pada baud rate `115200`.
4. Dekatkan objek ke sensor infrared.
5. Tunjukkan data masuk ke log HTTP dan MQTT.
6. Jelaskan bahwa HTTP lebih mudah dibuat, sedangkan MQTT lebih ringan untuk komunikasi IoT berbasis publish/subscribe.
