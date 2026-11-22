#include "mqtt.h"
#include "device_network.h"
#include "actuators.h"
#include <WiFi.h>
#include <PubSubClient.h>

static WiFiClient wifiClient;
static PubSubClient mqtt(wifiClient);

static bool alarmMuted = false;
static bool fanManualMode = false;
static bool panicMode = false;
static bool fanState = false;
static unsigned long lastMQTTAttempt = 0;
static String configuredMQTTHost;

static const char* STATE_TOPIC = "cinder/esp32/actuators";

static String readPayload(byte* payload, unsigned int length) {
  String value;
  value.reserve(length);

  for (unsigned int i = 0; i < length; i++) {
    value += (char)payload[i];
  }

  value.trim();
  return value;
}

static void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String topicText(topic);
  String command = readPayload(payload, length);

  if (topicText == "cinder/esp32/commands/mute") {
    if (command == "SILENCIAR") {
      alarmMuted = true;
      setAlarmMuted(true);
    } else if (command == "REATIVAR") {
      alarmMuted = false;
      setAlarmMuted(false);
    }
  } else if (topicText == "cinder/esp32/commands/fan") {
    if (command == "LIGAR") {
      fanManualMode = true;
      fanState = true;
      controlFan(true);
    } else if (command == "DESLIGAR") {
      fanManualMode = true;
      fanState = false;
      controlFan(false);
    } else if (command == "AUTO") {
      fanManualMode = false;
    }
  } else if (topicText == "cinder/esp32/commands/panic") {
    if (command == "ATIVAR") {
      panicMode = true;
      fanManualMode = true;
      fanState = true;
      controlFan(true);
      startAlarm();
      publishAlert("PANICO_ACIONADO");
    } else if (command == "DESATIVAR") {
      panicMode = false;
      fanManualMode = false;
      stopAlarm();
    }
  }

  mqtt.publish(STATE_TOPIC, "{\"updated\":true}", true);
}

static bool connectMQTT() {
  if (!networkConnected()) {
    return false;
  }

  configuredMQTTHost = getMQTTHost();
  mqtt.setServer(configuredMQTTHost.c_str(), getMQTTPort());
  mqtt.setCallback(mqttCallback);
  mqtt.setBufferSize(512);

  bool connected = mqtt.connect(
    "cinder-esp32",
    STATE_TOPIC,
    1,
    true,
    "{\"connection\":\"OFFLINE\"}"
  );

  if (!connected) {
    return false;
  }

  mqtt.subscribe("cinder/esp32/commands/#", 1);
  mqtt.publish(STATE_TOPIC, "{\"connection\":\"ONLINE\"}", true);
  return true;
}

void initializeMQTT() {
  lastMQTTAttempt = 0;
  connectMQTT();
}

void maintainMQTT() {
  if (mqtt.connected()) {
    mqtt.loop();
    return;
  }

  unsigned long now = millis();
  if (now - lastMQTTAttempt >= 5000) {
    lastMQTTAttempt = now;
    connectMQTT();
  }
}

bool mqttConnected() {
  return mqtt.connected();
}

bool panicModeActive() {
  return panicMode;
}

bool fanManualModeActive() {
  return fanManualMode;
}

void publishTelemetry(const EnvironmentReadings& readings) {
  if (!mqttConnected()) {
    return;
  }

  char payload[512];
  snprintf(
    payload, sizeof(payload),
    "{\"temperature\":%.2f,\"temperature_valid\":%s,"
    "\"voltage_mq135\":%.2f,\"co2\":%.2f,\"co_mq135\":%.2f,"
    "\"alcohol\":%.2f,\"nh4\":%.2f,\"toluene\":%.2f,"
    "\"acetone\":%.2f,\"voltage_mq4\":%.2f,\"ch4\":%.2f,"
    "\"lpg\":%.2f,\"propane\":%.2f,\"h2\":%.2f,\"smoke\":%.2f}",
    readings.temperature,
    readings.temperature_valid ? "true" : "false",
    readings.voltage_mq135, readings.co2, readings.co_mq135,
    readings.alcohol, readings.nh4, readings.toluene, readings.acetone,
    readings.voltage_mq4, readings.ch4, readings.lpg, readings.propane,
    readings.h2, readings.smoke
  );

  mqtt.publish("cinder/esp32/telemetry", payload, false);
}

void publishButtons(bool panicPressed, bool mutePressed) {
  if (!mqttConnected()) {
    return;
  }

  char payload[128];
  snprintf(payload, sizeof(payload),
    "{\"panic\":\"%s\",\"mute\":\"%s\"}",
    panicPressed ? "PRESSED" : "RELEASED",
    mutePressed ? "PRESSED" : "RELEASED"
  );

  mqtt.publish("cinder/esp32/buttons", payload, false);
}

void publishActuatorStates(const char* risk) {
  if (!mqttConnected()) {
    return;
  }

  char payload[256];
  snprintf(payload, sizeof(payload),
    "{\"risk\":\"%s\",\"green_led\":\"%s\","
    "\"yellow_led\":\"%s\",\"red_led\":\"%s\","
    "\"buzzer\":\"%s\",\"fan\":\"%s\","
    "\"fan_mode\":\"%s\",\"panic\":%s,\"connection\":\"ONLINE\"}",
    risk,
    strcmp(risk, "NORMAL") == 0 ? "ON" : "OFF",
    strcmp(risk, "ATENCAO") == 0 ? "ON" : "OFF",
    strcmp(risk, "PERIGO") == 0 ? "ON" : "OFF",
    alarmMuted ? "OFF" : "ON",
    fanIsOn() ? "ON" : "OFF",
    fanManualMode ? "MANUAL" : "AUTO",
    panicMode ? "true" : "false"
  );

  mqtt.publish(STATE_TOPIC, payload, true);
}

void publishAlert(const char* alertType) {
  if (!mqttConnected()) {
    return;
  }

  char payload[128];
  snprintf(payload, sizeof(payload), "{\"type\":\"%s\"}", alertType);
  mqtt.publish("cinder/esp32/alerts", payload, false);
}
