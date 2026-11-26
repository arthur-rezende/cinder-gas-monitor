# broker, tópicos, limiares (via .env)

import os
from pathlib import Path

from dotenv import load_dotenv

BASE_DIR = Path(__file__).resolve().parent
load_dotenv(BASE_DIR / ".env")


def _float(nome: str, padrao: float) -> float:
    return float(os.getenv(nome, padrao))


# Broker
MQTT_HOST = os.getenv("MQTT_HOST", "localhost")
MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))
MQTT_USER = os.getenv("MQTT_USER") or None
MQTT_PASSWORD = os.getenv("MQTT_PASSWORD") or None

# Tópicos
MQTT_BASE_TOPIC = os.getenv("MQTT_BASE_TOPIC", "cinder")
DEVICE_ID = os.getenv("DEVICE_ID", "esp32")

# Memória (integrar banco depois)
MAX_LEITURAS = int(os.getenv("MAX_LEITURAS", "720"))
MAX_ALERTAS = int(os.getenv("MAX_ALERTAS", "100"))

# Dashboard
DASHBOARD_REFRESH_S = int(os.getenv("DASHBOARD_REFRESH_5", "5"))
TELEMETRIA_TIMEOUT_S = int(os.getenv("TELEMETRIA_TIMEOUT_S", "20"))

# Limiares: cópia de firmware/firmware_device/config.h
# Quem decide o risco é o firmware.
LIMIARES = {
    "temperature": {"atencao": _float("TEMP_WARNING", 30.0), "perigo": _float("TEMP_DANGER", 40.0)},
    "co2":         {"atencao": _float("CO2_WARNING", 800.0), "perigo": _float("CO2_DANGER", 1500.0)},
    "ch4":         {"atencao": _float("CH4_WARNING", 200.0), "perigo": _float("CH4_DANGER", 800.0)},
    "smoke":       {"atencao": _float("SMOKE_WARNING", 100.0), "perigo": _float("SMOKE_DANGER", 500.0)},
}