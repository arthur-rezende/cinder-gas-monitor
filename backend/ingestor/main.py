import logging

from backend import config
from backend.data.repository import MemoryRepository
from backend.ingestor.handlers import make_handlers
from backend.ingestor.mqtt_client import CinderMQTTClient

log = logging.getLogger(__name__)


def main() -> None:
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s %(levelname)-7s %(name)s: %(message)s",
        datefmt="%H:%M:%S",
    )
    repo = MemoryRepository(config.MAX_LEITURAS, config.MAX_ALERTAS)
    cliente = CinderMQTTClient(
        config.MQTT_HOST, config.MQTT_PORT,
        username=config.MQTT_USER, password=config.MQTT_PASSWORD,
        base_topic=config.MQTT_BASE_TOPIC,
    )
    for tipo, handler in make_handlers(repo).items():
        cliente.register(tipo, handler)

    try:
        cliente.start()
    except KeyboardInterrupt:
        log.info("Encerrando...")
        cliente.stop()

if __name__ == "__main__":
    main()