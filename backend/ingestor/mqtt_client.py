# conexão, reconexão, subscribe

import json
import logging
import uuid
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
            client_id: str | None = None,
            username: str | None = None,
            password: str | None = None,
            base_topic: str = "cinder",
    ):
        self.host = host
        self.port = port
        self.base_topic = base_topic
        self._handlers: dict[str, Handler] = {}

        # paho-mqtt
        client_id = client_id or f"cinder-backend-{uuid.uuid4().hex[:6]}"
        self.client = mqtt.Client(
            mqtt.CallbackAPIVersion.VERSION2, client_id=client_id
        )
        if username:
            self.client.username_pw_set(username, password)

        # Reconexão automática
        self.client.on_connect = self._on_connect
        self.client.on_connect_fail = self._on_connect_fail
        self.client.on_disconnect = self._on_disconnect
        self.client.on_message = self._on_message

    def register(self, tipo: str, handler: Handler) -> None:
        self._handlers[tipo] = handler

    def start(self) -> None:
        # Não bloqueante
        log.info("Conectando a %s:%s", self.host, self.port)
        self.client.connect_async(self.host, self.port, keepalive=60)
        self.client.loop_forever(retry_first_connection=True)

    def start_background(self) -> None:
        # bloqueante
        log.info("Conectando a %s:%s (em segundo plano)", self.host, self.port)
        self.client.connect_async(self.host, self.port, keepalive=60)
        self.client.loop_start()

    def stop(self) -> None:
        self.client.disconnect()
        self.client.loop_stop()

    @property
    def conectado(self) -> bool:
        return self.client.is_connected()

    def publish_command(self, device_id: str, comando: str, valor: str) -> bool:
        # Publica em <base>/<device>/commands/<comando>
        topic = f"{self.base_topic}/{device_id}/commands/{comando}"
        info = self.client.publish(topic, valor, qos=1)
        log.info("Comando %s -> %s", topic, valor)
        return info.rc == mqtt.MQTT_ERR_SUCCESS

    # Callbacks internos

    def _on_connect(self, client, userdata, flags, reason_code, properties):
        if reason_code.is_failure:
            log.error("Falha ao conectar: %s", reason_code)
            return
        log.info("Conectado ao broker")
        #Assinar aqui
        for tipo in self._handlers:
            topic = f"{self.base_topic}/+/{tipo}"
            client.subscribe(topic, qos=1)
            log.info("Assinado: %s", topic)

    def _on_connect_fail(self, client, userdata):
        log.warning("Broker %s:%s indisponível. Tentando de novo...", self.host, self.port)

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
        if not isinstance(payload, dict):
            payload = {"valor": payload}

        try:
            handler(device_id, payload)
        except Exception:
            log.exception("Erro no handler '%s' (device %s)", tipo, device_id)