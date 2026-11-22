#pragma once
#include <Arduino.h>
#include "sensors.h"

void initializeMQTT();
void maintainMQTT();
bool mqttConnected();

void publishTelemetry(const EnvironmentReadings& readings);
void publishButtons(bool panicPressed, bool mutePressed);
void publishActuatorStates(const char* risk);
void publishAlert(const char* alertType);
bool panicModeActive();
bool fanManualModeActive();
