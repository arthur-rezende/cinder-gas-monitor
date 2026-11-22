#include "config.h"
#include "actuators.h"
#include "sensors.h"
#include "music.h"
#include "device_network.h"
#include "mqtt.h"

void debugMode();

enum RiskLevel {
  RISK_NORMAL,
  RISK_ATTENTION,
  RISK_DANGER
};

static unsigned long lastSensorRead = 0;
static unsigned long lastActuatorPublish = 0;
static unsigned long lastButtonPublish = 0;

static unsigned long panicStableAt = 0;
static unsigned long muteStableAt = 0;
static bool panicReading = false;
static bool panicPressed = false;
static bool muteReading = false;
static bool mutePressed = false;
static bool physicalPanicMode = false;
static unsigned long muteUntil = 0;
static const unsigned long PORTAL_COMBO_HOLD_MS = 2000;
static bool comboPressedActive = false;
static bool comboTriggered = false;
static unsigned long comboStartedAt = 0;

static RiskLevel currentRisk = RISK_NORMAL;
static EnvironmentReadings latestReadings;
static RiskLevel lastPublishedRisk = RISK_NORMAL;

static const char* riskName(RiskLevel risk) {
  if (risk == RISK_DANGER) return "PERIGO";
  if (risk == RISK_ATTENTION) return "ATENCAO";
  return "NORMAL";
}

static void updateButtonStates() {
  bool panicNow = button1Pressed();
  bool muteNow = button2Pressed();

  if (panicNow != panicReading) {
    panicReading = panicNow;
    panicStableAt = millis();
  }

  if (muteNow != muteReading) {
    muteReading = muteNow;
    muteStableAt = millis();
  }

  unsigned long now = millis();

  if (panicNow == panicReading &&
      now - panicStableAt >= BUTTON_DEBOUNCE_MS &&
      panicPressed != panicReading) {
    panicPressed = panicReading;

    if (!panicPressed && !comboPressedActive) {
      physicalPanicMode = !physicalPanicMode;

      if (physicalPanicMode) {
        startAlarm();
        controlFan(true);
        publishAlert("PANICO_ACIONADO");
      } else {
        stopAlarm();
        controlFan(false);
        publishAlert("RISCO_NORMALIZADO");
      }

      publishButtons(true, mutePressed);
    }
  }

  if (muteNow == muteReading &&
      now - muteStableAt >= BUTTON_DEBOUNCE_MS &&
      mutePressed != muteReading) {
    mutePressed = muteReading;

    if (!mutePressed && !comboPressedActive) {
      setAlarmMuted(true);
      muteUntil = now + ALARM_MUTE_MS;
      publishAlert("ALARM_MUTED");
      publishButtons(panicPressed, true);
    }
  }

  if (panicPressed && mutePressed) {
    if (!comboPressedActive) {
      comboPressedActive = true;
      comboTriggered = false;
      comboStartedAt = now;
    } else if (!comboTriggered && now - comboStartedAt >= PORTAL_COMBO_HOLD_MS) {
      comboTriggered = true;
      turnOnLight(PIN_BLUE);
      resetAndStartNetworkConfigurationPortal();
    }
  } else if (!panicPressed && !mutePressed) {
    comboPressedActive = false;
    comboTriggered = false;
  }

  if (alarmIsMuted() && now >= muteUntil) {
    setAlarmMuted(false);
  }
}

static RiskLevel calculateRisk(const EnvironmentReadings& readings) {
  if ((readings.temperature_valid && readings.temperature >= TEMP_DANGER) ||
      readings.co2 >= CO2_DANGER ||
      readings.ch4 >= CH4_DANGER ||
      readings.smoke >= SMOKE_DANGER) {
    return RISK_DANGER;
  }

  if ((readings.temperature_valid && readings.temperature >= TEMP_WARNING) ||
      readings.co2 >= CO2_WARNING ||
      readings.ch4 >= CH4_WARNING ||
      readings.smoke >= SMOKE_WARNING) {
    return RISK_ATTENTION;
  }

  return RISK_NORMAL;
}

static void updateRiskOutputs(RiskLevel risk) {
  if (physicalPanicMode || panicModeActive()) {
    turnOnLight(PIN_RED);
    startAlarm();
    controlFan(true);
    return;
  }

  if (risk == RISK_DANGER) {
    turnOnLight(PIN_RED);
    startAlarm();
    controlFan(true);
  } else if (risk == RISK_ATTENTION) {
    turnOnLight(PIN_BLUE);
    stopAlarm();

    if (!fanManualModeActive()) {
      controlFan(false);
    }
  } else {
    turnOnLight(PIN_GREEN);
    stopAlarm();

    if (!fanManualModeActive()) {
      controlFan(false);
    }
  }
}

void setup() {
  Serial.begin(115200);

  initializeActuators();
  initializeSensors();

  latestReadings = readAllSensors(false);

  if (initializeNetwork()) {
    initializeMQTT();
  }

  updateRiskOutputs(RISK_NORMAL);
  panicReading = button1Pressed();
  panicPressed = panicReading;
  muteReading = button2Pressed();
  mutePressed = muteReading;
  panicStableAt = millis();
  muteStableAt = millis();
}

void loop() {
  maintainNetwork();
  maintainMusic();

  if (networkConnected()) {
    maintainMQTT();
  }

  updateButtonStates();
  maintainAlarm();

  unsigned long now = millis();

  if (now - lastSensorRead >= SENSOR_INTERVAL_MS) {
    lastSensorRead = now;

    latestReadings = readAllSensors(false);
    RiskLevel nextRisk = calculateRisk(latestReadings);

    if (nextRisk != currentRisk) {
      if (nextRisk == RISK_DANGER) {
        publishAlert("GAS_CRITICO");
      } else if (nextRisk == RISK_ATTENTION) {
        publishAlert("GAS_ELEVADO");
      } else {
        publishAlert("RISCO_NORMALIZADO");
      }

      currentRisk = nextRisk;
    }

    updateRiskOutputs(currentRisk);
    publishTelemetry(latestReadings);
  }

  if (mqttConnected() && now - lastActuatorPublish >= MQTT_ACTUATOR_INTERVAL_MS) {
    lastActuatorPublish = now;

    if (physicalPanicMode || panicModeActive()) {
      publishActuatorStates("PERIGO");
    } else {
      publishActuatorStates(riskName(currentRisk));
    }

    lastPublishedRisk = physicalPanicMode ? RISK_DANGER : currentRisk;
  }

  if (now - lastButtonPublish >= 30000) {
    lastButtonPublish = now;
    publishButtons(panicPressed, mutePressed);
  }
}

void debugMode() {
  readAllSensors(true);
}