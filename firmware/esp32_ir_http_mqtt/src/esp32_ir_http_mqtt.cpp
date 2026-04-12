#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <WiFi.h>

#if __has_include("config.h")
#include "config.h"
#else
const char* WIFI_SSID = "NamaWiFi";
const char* WIFI_PASSWORD = "PasswordWiFi";
const char* HTTP_SERVER_URL = "http://IP-LAPTOP:8000/sensor";
const char* MQTT_BROKER_HOST = "IP-LAPTOP";
#endif

const char* DEVICE_ID = "esp32-ir-01";
const int MQTT_BROKER_PORT = 1883;
const char* MQTT_TOPIC = "iot/sensor/esp32-ir-01";

const int IR_SENSOR_PIN = 4;
const bool IR_ACTIVE_LOW = true;

const unsigned long SEND_INTERVAL_MS = 5000;
const unsigned long IR_DEBOUNCE_MS = 300;
const unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000;
const unsigned long WIFI_RETRY_INTERVAL_MS = 5000;
const unsigned long MQTT_CONNECT_TIMEOUT_MS = 8000;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

unsigned long lastSendMs = 0;
unsigned long lastIrTriggerMs = 0;
unsigned long sequenceNumber = 0;
unsigned long detectionCount = 0;
bool sendWithMqtt = true;

const char* wifiStatusName(wl_status_t status) {
  switch (status) {
    case WL_IDLE_STATUS:
      return "IDLE";
    case WL_NO_SSID_AVAIL:
      return "NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED:
      return "SCAN_COMPLETED";
    case WL_CONNECTED:
      return "CONNECTED";
    case WL_CONNECT_FAILED:
      return "CONNECT_FAILED";
    case WL_CONNECTION_LOST:
      return "CONNECTION_LOST";
    case WL_DISCONNECTED:
      return "DISCONNECTED";
    default:
      return "UNKNOWN";
  }
}

bool connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  static unsigned long lastWifiAttemptMs = 0;
  unsigned long now = millis();
  if (lastWifiAttemptMs != 0 && now - lastWifiAttemptMs < WIFI_RETRY_INTERVAL_MS) {
    return false;
  }
  lastWifiAttemptMs = now;

  Serial.print("Connecting WiFi SSID: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.disconnect(true);
  delay(100);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startedMs = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startedMs < WIFI_CONNECT_TIMEOUT_MS) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  wl_status_t status = WiFi.status();
  if (status != WL_CONNECTED) {
    Serial.print("WiFi failed, status: ");
    Serial.println(wifiStatusName(status));
    Serial.println("ESP32 needs a 2.4 GHz WiFi network. Check SSID/password and laptop IP.");
    return false;
  }

  Serial.print("WiFi connected, ESP32 IP: ");
  Serial.println(WiFi.localIP());
  return true;
}

bool connectMqtt() {
  if (mqttClient.connected()) {
    return true;
  }

  mqttClient.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);

  Serial.print("Connecting MQTT");
  unsigned long startedMs = millis();
  while (!mqttClient.connected() && millis() - startedMs < MQTT_CONNECT_TIMEOUT_MS) {
    String clientId = String(DEVICE_ID) + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println();
      Serial.println("MQTT connected");
      return true;
    } else {
      Serial.print(".");
      delay(1000);
    }
  }

  Serial.println();
  Serial.print("MQTT failed, state: ");
  Serial.println(mqttClient.state());
  return false;
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
  JsonDocument doc;
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
  if (!connectMqtt()) {
    return;
  }

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
  if (!connectWiFi()) {
    delay(1000);
    return;
  }

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
