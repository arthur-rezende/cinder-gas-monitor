# um handler por tipo de tópico: dict cru -> modelo -> repositório
import logging

from backend.models import EstadoAtuadores, Evento, Leitura

log = logging.getLogger(__name__)


def make_handlers(repo) -> dict:
    def on_telemetry(device_id: str, payload: dict) -> None:
        leitura = Leitura(device_id=device_id, **payload)
        repo.salvar_leitura(leitura)
        log.info("[%s] CH4=%s CO2=%s fumaça=%s T=%s",
                 device_id, leitura.ch4, leitura.co2, leitura.smoke, leitura.temperature)

    def on_actuators(device_id: str, payload: dict) -> None:
        repo.salvar_estado(device_id, EstadoAtuadores(**payload))

    def on_alerts(device_id: str, payload: dict) -> None:
        evento = Evento(device_id=device_id, tipo=payload.get("type", "DESCONHECIDO"))
        repo.salvar_evento(evento)
        log.warning("[%s] ALERTA: %s", device_id, evento.tipo)

    # a chave é o último nível do tópico: cinder/+/<chave>
    return {"telemetry": on_telemetry, "actuators": on_actuators, "alerts": on_alerts}