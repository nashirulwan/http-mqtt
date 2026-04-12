from __future__ import annotations

import json
import os
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

import paho.mqtt.client as mqtt

BASE_DIR = Path(__file__).resolve().parents[1]
DATA_DIR = BASE_DIR / "data"
MQTT_LOG = DATA_DIR / "mqtt_sensor_data.jsonl"

BROKER_HOST = os.environ.get("IOT_MQTT_HOST", "127.0.0.1")
BROKER_PORT = int(os.environ.get("IOT_MQTT_PORT", "1883"))
TOPIC = os.environ.get("IOT_MQTT_TOPIC", "iot/sensor/#")


def utc_now_iso() -> str:
    return datetime.now(timezone.utc).isoformat()


def append_jsonl(path: Path, payload: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("a", encoding="utf-8") as handle:
        handle.write(json.dumps(payload, ensure_ascii=True) + "\n")


def on_connect(client, userdata, flags, reason_code, properties=None):
    print(f"Connected to MQTT broker with reason_code={reason_code}", flush=True)
    client.subscribe(TOPIC)
    print(f"Subscribed to {TOPIC}", flush=True)


def on_message(client, userdata, msg):
    raw_payload = msg.payload.decode("utf-8", errors="replace")
    try:
        parsed_payload = json.loads(raw_payload)
    except json.JSONDecodeError:
        parsed_payload = {"raw_payload": raw_payload}

    record = {
        "received_at": utc_now_iso(),
        "protocol": "mqtt",
        "topic": msg.topic,
        "qos": msg.qos,
        "retain": msg.retain,
        "data": parsed_payload,
    }
    append_jsonl(MQTT_LOG, record)
    print(json.dumps(record, ensure_ascii=True), flush=True)


def main() -> None:
    client = mqtt.Client(
        mqtt.CallbackAPIVersion.VERSION2,
        client_id="iot-local-subscriber",
    )
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(BROKER_HOST, BROKER_PORT, keepalive=60)
    client.loop_forever()


if __name__ == "__main__":
    main()
