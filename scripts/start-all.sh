#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
RUN_DIR="$ROOT_DIR/run"
LOG_DIR="$ROOT_DIR/logs"
DATA_DIR="$ROOT_DIR/data"

mkdir -p "$RUN_DIR" "$LOG_DIR" "$DATA_DIR" "$ROOT_DIR/mosquitto-data"

for name in mosquitto http mqtt-subscriber; do
  pid_file="$RUN_DIR/$name.pid"
  if [[ -f "$pid_file" ]] && kill -0 "$(cat "$pid_file")" 2>/dev/null; then
    echo "$name masih berjalan dengan PID $(cat "$pid_file"). Jalankan ./scripts/stop-all.sh dulu."
    exit 1
  fi
  rm -f "$pid_file"
done

cd "$ROOT_DIR"

start_background() {
  local name="$1"
  local log_file="$2"
  shift 2

  setsid nohup "$@" >"$log_file" 2>&1 </dev/null &
  echo $! >"$RUN_DIR/$name.pid"
}

start_background mosquitto "$LOG_DIR/mosquitto.log" \
  mosquitto -c "$ROOT_DIR/server/mosquitto.conf"
start_background http "$LOG_DIR/http.log" \
  python "$ROOT_DIR/server/http_server.py"
start_background mqtt-subscriber "$LOG_DIR/mqtt-subscriber.log" \
  python "$ROOT_DIR/server/mqtt_subscriber.py"

sleep 2

echo "Stack lokal sudah dijalankan."
echo "HTTP health: http://127.0.0.1:8000/health"
echo "HTTP sensor: http://IP-LAPTOP:8000/sensor"
echo "MQTT broker: IP-LAPTOP:1883"
echo "MQTT topic : iot/sensor/esp32-ir-01"
echo
echo "PID:"
for name in mosquitto http mqtt-subscriber; do
  echo "  $name $(cat "$RUN_DIR/$name.pid")"
done
