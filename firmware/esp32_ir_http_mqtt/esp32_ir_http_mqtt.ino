#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <WiFi.h>

// Ubah sesuai WiFi dan IP laptop.
const char* WIFI_SSID = "NamaWiFi";
const char* WIFI_PASSWORD = "PasswordWiFi";

const char* DEVICE_ID = "esp32-ir-01";
const char* HTTP_SERVER_URL = "http://IP-LAPTOP:8000/sensor";
const char* MQTT_BROKER_HOST = "IP-LAPTOP";
const int MQTT_BROKER_PORT = 1883;
const char* MQTT_TOPIC = "iot/sensor/esp32-ir-01";

const int IR_SENSOR_PIN = 4;
const bool IR_ACTIVE_LOW = true;

const unsigned long SEND_INTERVAL_MS = 5000;
const unsigned long IR_DEBOUNCE_MS = 300;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

unsigned long lastSendMs = 0;
unsigned long lastIrTriggerMs = 0;
unsigned long sequenceNumber = 0;
unsigned long detectionCount = 0;
bool sendWithMqtt = true;

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.print("Connecting WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("WiFi connected, ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void connectMqtt() {
  if (mqttClient.connected()) {
    return;
  }

  mqttClient.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);

  Serial.print("Connecting MQTT");
  while (!mqttClient.connected()) {
    String clientId = String(DEVICE_ID) + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println();
      Serial.println("MQTT connected");
    } else {
      Serial.print(".");
      delay(1000);
    }
  }
}

bool isIrDetected(int rawValue) {
  if (IR_ACTIVE_LOW) {
    return rawValue == LOW;
  }
  return rawValue == HIGH;
}

void updateInfraredCounter(bool detected) {
  unsigned long now = millis();
  if (detected && now - lastIrTriggerMs >= IR_DEBOUNCE_MS) {
    detectionCount++;
    lastIrTriggerMs = now;
    Serial.print("IR detected, count: ");
    Serial.println(detectionCount);
  }
}

String buildPayload(const char* protocolName, int rawValue, bool detected) {
  StaticJsonDocument<256> doc;
  doc["device_id"] = DEVICE_ID;
  doc["sensor"] = "infrared";
  doc["ir_detected"] = detected;
  doc["ir_value"] = rawValue;
  doc["detection_count"] = detectionCount;
  doc["sequence"] = sequenceNumber;
  doc["protocol"] = protocolName;
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["uptime_ms"] = millis();

  String payload;
  serializeJson(doc, payload);
  return payload;
}

void sendHttp(const String& payload) {
  HTTPClient http;
  unsigned long startedUs = micros();

  http.begin(HTTP_SERVER_URL);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(payload);
  String response = http.getString();
  http.end();

  unsigned long durationUs = micros() - startedUs;
  Serial.print("HTTP POST ");
  Serial.print(httpCode);
  Serial.print(" ");
  Serial.print(durationUs / 1000.0, 2);
  Serial.print(" ms -> ");
  Serial.println(response);
}

void sendMqtt(const String& payload) {
  connectMqtt();

  unsigned long startedUs = micros();
  bool ok = mqttClient.publish(MQTT_TOPIC, payload.c_str());
  unsigned long durationUs = micros() - startedUs;

  Serial.print("MQTT publish ");
  Serial.print(ok ? "OK" : "FAILED");
  Serial.print(" ");
  Serial.print(durationUs / 1000.0, 2);
  Serial.print(" ms -> ");
  Serial.println(payload);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(IR_SENSOR_PIN, INPUT_PULLUP);

  Serial.println();
  Serial.println("ESP32 Infrared MQTT vs HTTP");
  connectWiFi();
  mqttClient.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
}

void loop() {
  connectWiFi();

  if (mqttClient.connected()) {
    mqttClient.loop();
  }

  int rawValue = digitalRead(IR_SENSOR_PIN);
  bool detected = isIrDetected(rawValue);
  updateInfraredCounter(detected);

  unsigned long now = millis();
  if (now - lastSendMs >= SEND_INTERVAL_MS) {
    lastSendMs = now;
    sequenceNumber++;

    if (sendWithMqtt) {
      String payload = buildPayload("mqtt", rawValue, detected);
      sendMqtt(payload);
    } else {
      String payload = buildPayload("http", rawValue, detected);
      sendHttp(payload);
    }

    sendWithMqtt = !sendWithMqtt;
  }
}
