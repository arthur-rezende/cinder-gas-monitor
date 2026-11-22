#pragma once
#include <Arduino.h>

struct EnvironmentReadings {
  // DS18B20
  float temperature;
  bool temperature_valid;

  // MQ-135
  float voltage_mq135;
  float co2;
  float co_mq135;
  float alcohol;
  float nh4;
  float toluene;
  float acetone;

  // MQ-4
  float voltage_mq4;
  float ch4;
  float lpg;
  float propane;
  float h2;
  float smoke;
};

void initializeSensors();
void calibrateMQSensors();
int countTemperatureDevices();
float readTemperature(bool printSerial = false);
EnvironmentReadings readAllSensors(bool printSerial = true);