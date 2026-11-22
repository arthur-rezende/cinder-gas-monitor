# Cinder Gas Monitor — Firmware ESP32

Firmware do dispositivo **Cinder Gas Monitor** para ESP32, responsável por leitura de sensores de gás e temperatura, acionamento local de atuadores (LED RGB, buzzer e relé do ventilador), portal de configuração Wi-Fi/MQTT e comunicação via protocolo **MQTT**.

---

## Sumário
- [Arquitetura do Código](#arquitetura-do-código)
- [Pinagem de Hardware](#pinagem-de-hardware)
- [Configuração Wi-Fi e Broker MQTT](#configuração-wi-fi-e-broker-mqtt)
- [Lógica de Risco e Limiares](#lógica-de-risco-e-limiares)
- [Guia de Integração MQTT (para a Aplicação)](#guia-de-integração-mqtt-para-a-aplicação)
  - [Dados de Conexão](#dados-de-conexão)
  - [Tópicos Publicados pela ESP32 (App faz Subscribe)](#tópicos-publicados-pela-esp32-app-faz-subscribe)
  - [Tópicos de Comando (App faz Publish)](#tópicos-de-comando-app-faz-publish)
  - [Observações Importantes para o Backend / Frontend](#observações-importantes-para-o-backend--frontend)
- [Compilação e Dependências](#compilação-e-dependências)

---

## Arquitetura do Código

| Arquivo | Responsabilidade |
| :--- | :--- |
| `firmware_device.ino` | `setup()` e `loop()` principais, cálculo de risco, controle dos botões físicos e coordenação das publicações MQTT. |
| `config.h` | Mapeamento de pinos, constantes de calibração ADC/MQ, limiares de atenção/perigo e intervalos de tempo. |
| `sensors.h` / `sensors.cpp` | Calibração e leitura do sensor de temperatura **DS18B20** e dos sensores de gás **MQ-135** e **MQ-4**. |
| `actuators.h` / `actuators.cpp` | Controle do LED RGB (ânodo comum), buzzer (alarme intermitente) e relé do ventilador. |
| `device_network.h` / `device_network.cpp` | Gerenciamento Wi-Fi com `WiFiManager` em modo não-bloqueante e persistência de host/porta MQTT via `Preferences`. |
| `mqtt.h` / `mqtt.cpp` | Conexão MQTT (`PubSubClient`), envio de telemetria/atuadores/alertas/botões e processamento de comandos remotos. |
| `music.h` / `music.cpp` / `notes.h` | Rotina não-bloqueante de teste sonoro no buzzer (*Asa Branca*). |

---

## Pinagem de Hardware

Definida em `config.h`:

| Função | GPIO | Observação |
| :--- | :---: | :--- |
| `PIN_RED` | `21` | LED RGB (ânodo comum — `LOW` acende) |
| `PIN_GREEN` | `19` | LED RGB (ânodo comum — `LOW` acende) |
| `PIN_BLUE` | `18` | LED RGB (ânodo comum — `LOW` acende) |
| `PIN_BUZZER` | `15` | Buzzer (`tone` em `1200 Hz`, ciclo `250 ms` ligado / `500 ms` desligado) |
| `PIN_RELE` | `25` | Módulo relé do ventilador (ativo em `LOW`) |
| `PIN_BTN_1` | `16` | Botão Pânico (alterna liga/desliga ao soltar) |
| `PIN_BTN_2` | `17` | Botão Silenciar alarme por `60 s` (aciona ao soltar) |
| `PIN_ONE_WIRE` | `4` | Sensor de temperatura DS18B20 |
| `PIN_MQ4` | `34` | Sensor MQ-4 (gases combustíveis / fumaça) |
| `PIN_MQ135` | `35` | Sensor MQ-135 (qualidade do ar / CO₂ / etc.) |

### Atalho dos Botões Físicos
- **Botão 1 (`GPIO 16`) sozinho:** ativa/desativa o modo de pânico físico (liga LED vermelho, buzzer e ventilador).
- **Botão 2 (`GPIO 17`) sozinho:** silencia o buzzer por `60.000 ms` (`ALARM_MUTE_MS`).
- **Botão 1 + Botão 2 juntos por `2 segundos`:** acende o LED azul, apaga as credenciais Wi-Fi salvas e abre o portal de configuração `Cinder-Gas-Monitor`.

---

## Configuração Wi-Fi e Broker MQTT

O firmware usa `WiFiManager` em **modo não-bloqueante** (`setConfigPortalBlocking(false)`), permitindo que sensores, botões e alarme continuem operando mesmo enquanto o portal estiver aberto.

- **Hostname na rede:** `cinder-esp32`
- **Access Point do portal:**
  - **SSID:** `Cinder-Gas-Monitor`
  - **Senha:** `cinder123`
  - **Timeout de tentativa de conexão no boot:** `10 s`
  - **Timeout do portal sem clientes conectados:** `180 s`
- **Parâmetros salvos no `Preferences` (namespace `"cinder"`):**
  - `mqtt_host` (padrão: `"192.168.1.10"`, máximo 64 caracteres)
  - `mqtt_port` (padrão: `1883`)

---

## Lógica de Risco e Limiares

A cada `5.000 ms` (`SENSOR_INTERVAL_MS`), `calculateRisk()` avalia os limiares definidos em `config.h`:

| Variável | Atenção (`ATENCAO`) | Perigo (`PERIGO`) |
| :--- | :---: | :---: |
| Temperatura (`DS18B20`, se válida) | `>= 30.0 °C` | `>= 40.0 °C` |
| CO₂ (`MQ-135`, base + `400.0`) | `>= 800.0 ppm` | `>= 1500.0 ppm` |
| CH₄ (`MQ-4`) | `>= 200.0 ppm` | `>= 800.0 ppm` |
| Fumaça (`MQ-4`) | `>= 100.0 ppm` | `>= 500.0 ppm` |

- **`NORMAL` (`RISK_NORMAL`):** LED verde aceso, alarme desligado, ventilador desligado (se em modo `AUTO`).
- **`ATENCAO` (`RISK_ATTENTION`):** LED azul aceso, alarme desligado, ventilador desligado (se em modo `AUTO`).
- **`PERIGO` (`RISK_DANGER`) ou Pânico:** LED vermelho aceso, alarme tocando (se não silenciado) e ventilador ligado.

---

## Guia de Integração MQTT (para a Aplicação)

### Dados de Conexão
- **Client ID da ESP32:** `cinder-esp32`
- **Protocolo:** MQTT sobre TCP puro (sem TLS).
- **Porta padrão:** `1883` *(ex.: para testes no broker público do HiveMQ, usar host `broker.hivemq.com` e porta `1883`)*.
- **Tamanho do buffer:** `512` bytes.
- **Last Will and Testament (LWT):**
  - **Tópico:** `cinder/esp32/actuators`
  - **QoS:** `1` | **Retain:** `true`
  - **Payload se desconectar inesperadamente:** `{"connection":"OFFLINE"}`

> **Atenção em brokers públicos:** como o `clientId` é fixo (`"cinder-esp32"`), apenas uma ESP32 pode estar conectada por vez usando o mesmo broker público.

---

### Tópicos Publicados pela ESP32 (App faz Subscribe)

Sugestão para assinar tudo de uma vez durante desenvolvimento: `cinder/esp32/#`

#### 1. `cinder/esp32/telemetry`
- **Frequência:** a cada `5 segundos`
- **Retain:** `false`
- **Formato:** JSON com leituras dos sensores

```json
{
  "temperature": 25.50,
  "temperature_valid": true,
  "voltage_mq135": 0.36,
  "co2": 400.32,
  "co_mq135": 0.19,
  "alcohol": 0.11,
  "nh4": 0.65,
  "toluene": 0.04,
  "acetone": 0.04,
  "voltage_mq4": 0.55,
  "ch4": 2.81,
  "lpg": 5.30,
  "propane": 6.75,
  "h2": 10.25,
  "smoke": 0.71
}
```

| Campo | Tipo | Descrição |
| :--- | :--- | :--- |
| `temperature` | `number` | Temperatura em °C (`-127.00` quando desconectado) |
| `temperature_valid` | `boolean` | `true` se o DS18B20 respondeu normalmente |
| `voltage_mq135` | `number` | Tensão lida no pino do MQ-135 (V) |
| `co2` | `number` | Estimativa de CO₂ em ppm (inclui baseline `+400.0`) |
| `co_mq135` | `number` | Estimativa de CO pelo MQ-135 (ppm) |
| `alcohol` | `number` | Estimativa de álcool (ppm) |
| `nh4` | `number` | Estimativa de amônia / NH₄ (ppm) |
| `toluene` | `number` | Estimativa de tolueno (ppm) |
| `acetone` | `number` | Estimativa de acetona (ppm) |
| `voltage_mq4` | `number` | Tensão lida no pino do MQ-4 (V) |
| `ch4` | `number` | Estimativa de metano / CH₄ (ppm) |
| `lpg` | `number` | Estimativa de GLP (ppm) |
| `propane` | `number` | Estimativa de propano (ppm) |
| `h2` | `number` | Estimativa de hidrogênio (ppm) |
| `smoke` | `number` | Estimativa de fumaça (ppm) |

---

#### 2. `cinder/esp32/actuators`
- **Frequência:** a cada `5 segundos`, além de eventos de conexão/LWT e resposta a comandos
- **Retain:** `true`
- **Formato completo (periódico a cada 5s):**

```json
{
  "risk": "NORMAL",
  "green_led": "ON",
  "yellow_led": "OFF",
  "red_led": "OFF",
  "buzzer": "ON",
  "fan": "OFF",
  "fan_mode": "AUTO",
  "panic": false,
  "connection": "ONLINE"
}
```

| Campo | Valores possíveis | Descrição |
| :--- | :--- | :--- |
| `risk` | `"NORMAL"`, `"ATENCAO"`, `"PERIGO"` | Nível atual calculado (ou `"PERIGO"` se pânico ativo) |
| `green_led` | `"ON"`, `"OFF"` | `"ON"` quando `risk == "NORMAL"` |
| `yellow_led` | `"ON"`, `"OFF"` | `"ON"` quando `risk == "ATENCAO"` *(no hardware atual acende o pino azul do LED RGB)* |
| `red_led` | `"ON"`, `"OFF"` | `"ON"` quando `risk == "PERIGO"` |
| `buzzer` | `"ON"`, `"OFF"` | Reflete `!alarmMuted` (indica se o alarme está habilitado ou silenciado) |
| `fan` | `"ON"`, `"OFF"` | Estado atual do ventilador (`fanIsOn()`) |
| `fan_mode` | `"AUTO"`, `"MANUAL"` | Modo de controle do ventilador |
| `panic` | `true`, `false` | Estado do modo pânico acionado via comando MQTT |
| `connection` | `"ONLINE"`, `"OFFLINE"` | Status de conexão do dispositivo |

**Outros payloads possíveis neste mesmo tópico (`cinder/esp32/actuators`):**
- Ao conectar: `{"connection":"ONLINE"}`
- LWT ao cair conexão: `{"connection":"OFFLINE"}`
- Confirmação imediata após receber comando em `cinder/esp32/commands/#`: `{"updated":true}`

---

#### 3. `cinder/esp32/alerts`
- **Frequência:** sob evento (mudança de risco ou acionamento de botão/pânico)
- **Retain:** `false`
- **Formato:**

```json
{
  "type": "GAS_CRITICO"
}
```

| Valor de `type` | Quando é publicado |
| :--- | :--- |
| `"GAS_ELEVADO"` | Risco mudou para `ATENCAO` |
| `"GAS_CRITICO"` | Risco mudou para `PERIGO` |
| `"RISCO_NORMALIZADO"` | Risco voltou para `NORMAL` ou pânico físico foi desativado no botão |
| `"PANICO_ACIONADO"` | Pânico ativado pelo botão físico ou pelo comando MQTT `ATIVAR` |
| `"ALARM_MUTED"` | Alarme silenciado pelo botão físico (`PIN_BTN_2`) |

---

#### 4. `cinder/esp32/buttons`
- **Frequência:** ao acionar os botões físicos e periodicamente a cada `30 segundos`
- **Retain:** `false`
- **Formato:**

```json
{
  "panic": "RELEASED",
  "mute": "RELEASED"
}
```

| Campo | Valores possíveis |
| :--- | :--- |
| `panic` | `"PRESSED"`, `"RELEASED"` |
| `mute` | `"PRESSED"`, `"RELEASED"` |

---

### Tópicos de Comando (App faz Publish)

A ESP32 assina `cinder/esp32/commands/#` com **QoS 1**.  
Os payloads devem ser enviados como **texto puro** (*string simples*, sem JSON), exatamente em **MAIÚSCULAS**:

| Tópico | Payloads aceitos | Efeito na ESP32 |
| :--- | :--- | :--- |
| `cinder/esp32/commands/mute` | `SILENCIAR` | Silencia o alarme sonoro (`alarmMuted = true`) |
| `cinder/esp32/commands/mute` | `REATIVAR` | Reabilita o alarme sonoro (`alarmMuted = false`) |
| `cinder/esp32/commands/fan` | `LIGAR` | Coloca ventilador em `MANUAL` e liga |
| `cinder/esp32/commands/fan` | `DESLIGAR` | Coloca ventilador em `MANUAL` e desliga *(em `PERIGO`/pânico, religa no ciclo seguinte de 5s)* |
| `cinder/esp32/commands/fan` | `AUTO` | Retorna o ventilador para controle automático |
| `cinder/esp32/commands/panic` | `ATIVAR` | Ativa `panicMode`, força ventilador ligado, dispara alarme e publica alerta `PANICO_ACIONADO` |
| `cinder/esp32/commands/panic` | `DESATIVAR` | Desativa `panicMode`, volta ventilador para `AUTO` e para o alarme |

---

### Observações Importantes para o Backend / Frontend

1. **Payloads parciais em `cinder/esp32/actuators`:** o mesmo tópico recebe `{"connection":"ONLINE"}`, `{"connection":"OFFLINE"}`, `{"updated":true}` e o JSON completo a cada 5s. Na aplicação, faça **merge** dos campos recebidos em vez de sobrescrever todo o objeto de estado (ou verifique se `"risk"` existe antes de atualizar os indicadores).
2. **Campo `buzzer`:** no firmware atual, `"buzzer"` indica se o som está habilitado (`!alarmMuted`), então ele aparece `"ON"` mesmo em `NORMAL` e `ATENCAO` (quando o buzzer está fisicamente silencioso). Para saber se o alarme está efetivamente soando no ambiente, verifique `risk === "PERIGO" && buzzer === "ON"`.
3. **Campo `panic` vs `risk`:** se o usuário apertar o botão físico de pânico na caixa, `risk` muda para `"PERIGO"` (e `alerts` emite `PANICO_ACIONADO`), mas o campo booleano `"panic"` no payload de `actuators` só fica `true` quando o pânico foi acionado via MQTT.

---

## Compilação e Dependências

- **Placa / Core:** `esp32:esp32` (`3.3.12`) — FQBN `esp32:esp32:esp32`
- **Bibliotecas utilizadas:**
  - `WiFiManager` (`2.0.17`)
  - `PubSubClient` (`2.8`)
  - `OneWire` (`2.3.8`)
  - `DallasTemperature` (`4.0.6`)
  - `MQUnifiedsensor` (`3.0.0`)
  - Bibliotecas nativas do core ESP32 (`WiFi`, `WebServer`, `DNSServer`, `Preferences`)
