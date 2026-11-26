# lê do repository.py, não fala MQTT
# Dashboard: streamlit run backend/dashboard/app.py

import sys
from datetime import datetime
from pathlib import Path

# O streamlit run não põe a raiz do projeto no sys.path; sem isso, "import backend" falha
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))

import logging

import pandas as pd
import plotly.graph_objects as go
import streamlit as st

from backend import config
from backend.data.repository import MemoryRepository
from backend.ingestor.handlers import make_handlers
from backend.ingestor.mqtt_client import CinderMQTTClient

logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)-7s %(name)s: %(message)s")

st.set_page_config(page_title="Cinder Gas Monitor", page_icon="🔥", layout="wide")
DEVICE = config.DEVICE_ID
ICONE_RISCO = {"NORMAL": "🟢", "ATENCAO": "🟡", "PERIGO": "🔴"}


@st.cache_resource
def iniciar_backend():
    """Roda UMA vez por servidor Streamlit: cria o repositório e liga o MQTT em segundo plano."""
    repo = MemoryRepository(config.MAX_LEITURAS, config.MAX_ALERTAS)
    cliente = CinderMQTTClient(
        config.MQTT_HOST, config.MQTT_PORT,
        username=config.MQTT_USER, password=config.MQTT_PASSWORD,
        base_topic=config.MQTT_BASE_TOPIC,
    )
    for tipo, handler in make_handlers(repo).items():
        cliente.register(tipo, handler)
    cliente.start_background()
    return repo, cliente


repo, cliente = iniciar_backend()


def enviar(comando: str, valor: str) -> None:
    if cliente.publish_command(DEVICE, comando, valor):
        st.toast(f"Enviado: {comando} → {valor}")
    else:
        st.toast("Sem conexão com o broker", icon="⚠️")


def grafico(df: pd.DataFrame, campo: str, titulo: str, unidade: str) -> go.Figure:
    fig = go.Figure()
    fig.add_scatter(x=df["recebido_em"], y=df[campo], mode="lines", name=titulo)
    limiar = config.LIMIARES.get(campo)
    if limiar:
        fig.add_hline(y=limiar["atencao"], line_dash="dash", line_color="orange", annotation_text="Atenção")
        fig.add_hline(y=limiar["perigo"], line_dash="dash", line_color="red", annotation_text="Perigo")
    fig.update_layout(title=f"{titulo} ({unidade})", height=350, margin=dict(l=10, r=10, t=40, b=10))
    return fig


# ---------- Barra lateral: comandos (fora do fragment, não precisa atualizar sozinha) ----------
with st.sidebar:
    st.header("Comandos")
    st.caption(f"Dispositivo: `{DEVICE}`")

    st.subheader("Cooler")
    c1, c2, c3 = st.columns(3)
    if c1.button("Ligar"):
        enviar("fan", "LIGAR")
    if c2.button("Desligar"):
        enviar("fan", "DESLIGAR")
    if c3.button("Auto"):
        enviar("fan", "AUTO")

    st.subheader("Alarme")
    a1, a2 = st.columns(2)
    if a1.button("Silenciar"):
        enviar("mute", "SILENCIAR")
    if a2.button("Reativar"):
        enviar("mute", "REATIVAR")

    st.subheader("Pânico")
    if st.button("🚨 Ativar pânico", type="primary"):
        enviar("panic", "ATIVAR")
    if st.button("Desativar pânico"):
        enviar("panic", "DESATIVAR")


# ---------- Painel principal: redesenha a cada N segundos ----------
st.title("🔥 Cinder Gas Monitor")


@st.fragment(run_every=config.DASHBOARD_REFRESH_S)
def painel():
    if not cliente.conectado:
        st.error(f"Sem conexão com o broker {config.MQTT_HOST}:{config.MQTT_PORT}. Tentando reconectar...")

    estado = repo.estado_atual(DEVICE)
    ultima = repo.ultima_leitura(DEVICE)

    # Estado dos atuadores
    risco = estado.get("risk")
    s = st.columns(5)
    s[0].metric("ESP32", estado.get("connection", "—"))
    s[1].metric("Risco", f"{ICONE_RISCO.get(risco, '⚪')} {risco or '—'}")
    s[2].metric("Cooler", f"{estado.get('fan', '—')} ({estado.get('fan_mode', '—')})")
    s[3].metric("Buzzer", estado.get("buzzer", "—"))
    s[4].metric("Pânico", "ATIVO" if estado.get("panic") else "—")

    if ultima is None:
        st.info("Aguardando telemetria da ESP32...")
        return

    idade = (datetime.now() - ultima.recebido_em).total_seconds()
    if idade > config.TELEMETRIA_TIMEOUT_S:
        st.warning(f"Última telemetria há {idade:.0f} s: os dados podem estar desatualizados.")

    # Leituras atuais
    m = st.columns(4)
    m[0].metric("CH₄", f"{ultima.ch4:.1f} ppm" if ultima.ch4 is not None else "—")
    m[1].metric("CO₂", f"{ultima.co2:.0f} ppm" if ultima.co2 is not None else "—")
    m[2].metric("Fumaça", f"{ultima.smoke:.1f} ppm" if ultima.smoke is not None else "—")
    m[3].metric("Temperatura",
                f"{ultima.temperature:.1f} °C" if ultima.temperature_valid else "sensor inválido")

    # Histórico
    df = pd.DataFrame([l.model_dump() for l in repo.ultimas_leituras(DEVICE)])
    series = [("ch4", "CH₄", "ppm"), ("co2", "CO₂", "ppm"), ("smoke", "Fumaça", "ppm"),
              ("temperature", "Temperatura", "°C")]
    for aba, (campo, titulo, unidade) in zip(st.tabs([t for _, t, _ in series]), series):
        with aba:
            st.plotly_chart(grafico(df, campo, titulo, unidade), key=f"graf_{campo}")

    # Alertas
    st.subheader("Últimos alertas")
    eventos = repo.ultimos_eventos(DEVICE)
    if eventos:
        st.dataframe(
            pd.DataFrame([{"Horário": e.recebido_em.strftime("%H:%M:%S"), "Tipo": e.tipo} for e in eventos]),
            hide_index=True,
        )
    else:
        st.caption("Nenhum alerta recebido.")


painel()