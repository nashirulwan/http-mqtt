from __future__ import annotations

import json
import os
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

from flask import Flask, jsonify, request

app = Flask(__name__)

BASE_DIR = Path(__file__).resolve().parents[1]
DATA_DIR = BASE_DIR / "data"
HTTP_LOG = DATA_DIR / "http_sensor_data.jsonl"
HTTP_PORT = int(os.environ.get("IOT_HTTP_PORT", "8000"))


def utc_now_iso() -> str:
    return datetime.now(timezone.utc).isoformat()


def append_jsonl(path: Path, payload: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("a", encoding="utf-8") as handle:
        handle.write(json.dumps(payload, ensure_ascii=True) + "\n")


@app.get("/")
def index():
    return jsonify(
        {
            "service": "iot-http-api",
            "status": "ok",
            "endpoints": ["/health", "/sensor", "/update"],
        }
    )


@app.get("/health")
def health():
    return jsonify({"status": "ok", "time": utc_now_iso()})


@app.post("/sensor")
@app.post("/update")
def receive_sensor():
    if not request.is_json:
        return jsonify({"status": "error", "message": "JSON body required"}), 400

    body = request.get_json(silent=True)
    if not isinstance(body, dict):
        return jsonify({"status": "error", "message": "Invalid JSON object"}), 400

    record = {
        "received_at": utc_now_iso(),
        "protocol": "http",
        "remote_addr": request.headers.get("X-Forwarded-For", request.remote_addr),
        "data": body,
    }
    append_jsonl(HTTP_LOG, record)
    print(json.dumps(record, ensure_ascii=True), flush=True)

    return jsonify({"status": "ok", "message": "Sensor data stored", "record": record}), 200


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=HTTP_PORT)
