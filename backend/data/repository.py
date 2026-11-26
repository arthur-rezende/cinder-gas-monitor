# salvar leitura e ultimas leituras
# salvar leitura e ultimas leituras
# Por enquanto em memória: os dados somem ao reiniciar.
# O futuro repositório SQLite deve ter estes MESMOS métodos.

import threading
from collections import deque
from datetime import datetime

from backend.models import EstadoAtuadores, Evento, Leitura


class MemoryRepository:
    def __init__(self, max_leituras: int = 720, max_alertas: int = 100):
        self._lock = threading.Lock()
        self._max_leituras = max_leituras
        self._leituras: dict[str, deque[Leitura]] = {}
        self._eventos: deque[Evento] = deque(maxlen=max_alertas)
        self._estados: dict[str, dict] = {}

    # ---- escrita (handlers) ----
    def salvar_leitura(self, leitura: Leitura) -> None:
        with self._lock:
            fila = self._leituras.setdefault(leitura.device_id, deque(maxlen=self._max_leituras))
            fila.append(leitura)

    def salvar_estado(self, device_id: str, estado: EstadoAtuadores) -> None:
        with self._lock:
            atual = self._estados.setdefault(device_id, {})
            atual.update(estado.model_dump(exclude_none=True))   # mescla, não substitui

    def salvar_evento(self, evento: Evento) -> None:
        with self._lock:
            self._eventos.append(evento)

    # ---- leitura (dashboard) ----
    def ultimas_leituras(self, device_id: str) -> list[Leitura]:
        with self._lock:
            return list(self._leituras.get(device_id, ()))

    def ultima_leitura(self, device_id: str) -> Leitura | None:
        with self._lock:
            fila = self._leituras.get(device_id)
            return fila[-1] if fila else None

    def estado_atual(self, device_id: str) -> dict:
        with self._lock:
            return dict(self._estados.get(device_id, {}))   # cópia: quem lê não altera o original

    def ultimos_eventos(self, device_id: str, limite: int = 20) -> list[Evento]:
        with self._lock:
            eventos = [e for e in self._eventos if e.device_id == device_id]
        return list(reversed(eventos[-limite:]))   # mais recente primeiro