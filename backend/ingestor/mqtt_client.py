# conexão, reconexão, subscribe

import json
import logging
from typing import Callable

import paho.mqtt.client as mqtt

log = logging.getLogger(__name__)

# Assinatura de handler
Handler = Callable[[str, dict], None]


class CinderMQTTClient:
    def __init__(
            self,
            host: str,
            port: int = 1883,
            client_id: str = "cinder-ingestor",
            username: str | None = None,
            password: str | None = None,
            base_topic: str = "cinder",
    ):
        self.host = host
        self.port = port
        self.base_topic = base_topic
        self._handlers: dict[str, Handler] = {}

        # paho-mqtt
        self.client = mqtt.Client(
            mqtt.CallbackAPIVersion.VERSION2, client_id=client_id
        )
        if username:
            self.client.username_pw_set(username, password)

        # Reconexão automática
        self.client.on_connect = self._on_connect
        self.client.on_disconnect = self._on_disconnect
        self.client.on_message = self._on_message

    def register(self, tipo: str, handler: Handler) -> None:
        self._handlers[tipo] = handler

    def start(self) -> None:
        log.info("Conectando a %s:%s", self.host, self.port)
        self.client.connect(self.host, self.port, keepalive=60)
        self.client.loop_forever()

    def stop(self) -> None:
        self.client.disconnect()

    # Callbacks internos

    def _on_connect(self, client, userdata, flags, reason_code, properties):
        if reason_code.is_failure:
            log.error("Falha ao conectar: %s", reason_code)
            return
        log.info("Conectando ao broker")
        #Assinar aqui
        for tipo in self._handlers:
            topic = f"{self.base_topic}/+/{tipo}"
            client.subscribe(topic, qos=1)
            log.info("Assinado: %s", topic)

    def _on_disconnect(self, client, userdata, flags, reason_code, properties):
        log.warning("Desconectado (%s). Testando reconectar...", reason_code)

    def _on_message(self, client, userdata, msg: mqtt.MQTTMessage):
        # Formato: cinder/<device_id>/<tipo>
        parts = msg.topic.split("/")
        if len(parts) != 3:
            log.warning("Tópico fora ddo padrão: %s", msg.topic)
            return
        _, device_id, tipo = parts

        handler = self._handlers.get(tipo)
        if handler is None:
            return

        try:
            payload = json.loads(msg.payload.decode("utf-8"))
        except (UnicodeDecodeError, json.JSONDecodeError):
            payload = {"valor": msg.payload.decode("utf-8", errors="replace")}

        try:
            handler(device_id, payload)
        except Exception:
            log.exception("Erro no handler '%s' (device %s)", tipo, device_id)