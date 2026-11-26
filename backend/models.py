# dataClasses/Pydantic: Leitura, Evento

# dataClasses/Pydantic: Leitura, Evento

from datetime import datetime

from pydantic import BaseModel, ConfigDict, Field


class Leitura(BaseModel):
    """cinder/<device>/telemetry: a cada 5 s."""
    model_config = ConfigDict(extra="allow")   # campo novo no firmware não quebra nada

    device_id: str
    recebido_em: datetime = Field(default_factory=datetime.now)

    temperature: float | None = None
    temperature_valid: bool | None = None
    # MQ-135
    voltage_mq135: float | None = None
    co2: float | None = None
    co_mq135: float | None = None
    alcohol: float | None = None
    nh4: float | None = None
    toluene: float | None = None
    acetone: float | None = None
    # MQ-4
    voltage_mq4: float | None = None
    ch4: float | None = None
    lpg: float | None = None
    propane: float | None = None
    h2: float | None = None
    smoke: float | None = None


class EstadoAtuadores(BaseModel):
    """cinder/<device>/actuators (retained): pode chegar parcial."""
    model_config = ConfigDict(extra="ignore")  # descarta o {"updated": true}

    risk: str | None = None        # NORMAL | ATENCAO | PERIGO
    green_led: str | None = None
    yellow_led: str | None = None
    red_led: str | None = None
    buzzer: str | None = None      # ON | OFF
    fan: str | None = None         # ON | OFF
    fan_mode: str | None = None    # AUTO | MANUAL
    panic: bool | None = None
    connection: str | None = None  # ONLINE | OFFLINE


class Evento(BaseModel):
    """cinder/<device>/alerts"""
    device_id: str
    tipo: str   # GAS_CRITICO, GAS_ELEVADO, RISCO_NORMALIZADO, PANICO_ACIONADO, ALARM_MUTED
    recebido_em: datetime = Field(default_factory=datetime.now)