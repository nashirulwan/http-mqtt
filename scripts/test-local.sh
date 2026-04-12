#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

echo "== health =="
curl -s http://127.0.0.1:8000/health | jq .
echo

echo "== http test =="
curl -s -X POST http://127.0.0.1:8000/sensor \
  -H "Content-Type: application/json" \
  -d '{"device_id":"esp32-ir-01","sensor":"infrared","ir_detected":true,"ir_value":0,"detection_count":1,"sequence":1,"protocol":"http"}' | jq .
echo

echo "== mqtt test =="
mosquitto_pub -h 127.0.0.1 -p 1883 -t 'iot/sensor/esp32-ir-01' \
  -m '{"device_id":"esp32-ir-01","sensor":"infrared","ir_detected":true,"ir_value":0,"detection_count":2,"sequence":2,"protocol":"mqtt"}'
sleep 1

echo "== http log =="
tail -n 1 "$ROOT_DIR/data/http_sensor_data.jsonl"
echo

echo "== mqtt log =="
tail -n 1 "$ROOT_DIR/data/mqtt_sensor_data.jsonl"
