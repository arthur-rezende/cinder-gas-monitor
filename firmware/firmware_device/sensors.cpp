#include "sensors.h"
#include "config.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <MQUnifiedsensor.h>

static OneWire oneWire(PIN_ONE_WIRE);
static DallasTemperature sensors(&oneWire);

static MQUnifiedsensor MQ135(
  BOARD_NAME,
  VOLTAGE_RESOLUTION,
  ADC_BIT_RESOLUTION,
  PIN_MQ135,
  "MQ-135"
);

static MQUnifiedsensor MQ4(
  BOARD_NAME,
  VOLTAGE_RESOLUTION,
  ADC_BIT_RESOLUTION,
  PIN_MQ4,
  "MQ-4"
);

void initializeSensors() {
  sensors.begin();
  calibrateMQSensors();
}

void calibrateMQSensors() {
  Serial.println("Calibrating MQ sensors in clean air...");

  MQ135.setRegressionMethod(1);
  MQ135.setA(110.47);
  MQ135.setB(-2.862);
  MQ135.init();

  float r0MQ135 = 0;
  for (int i = 0; i < 10; i++) {
    MQ135.update();
    r0MQ135 += MQ135.calibrate(RATIO_MQ135_CLEAN_AIR);
  }
  MQ135.setR0(r0MQ135 / 10.0);

  MQ4.setRegressionMethod(1);
  MQ4.setA(1012.7);
  MQ4.setB(-2.786);
  MQ4.init();

  float r0MQ4 = 0;
  for (int i = 0; i < 10; i++) {
    MQ4.update();
    r0MQ4 += MQ4.calibrate(RATIO_MQ4_CLEAN_AIR);
  }
  MQ4.setR0(r0MQ4 / 10.0);

  if (isinf(r0MQ135) || isinf(r0MQ4)) {
    Serial.println("[WARNING] Infinite R0 detected. Check MQ sensor connections.");
  } else {
    Serial.println("MQ sensor calibration complete.");
  }
}

int countTemperatureDevices() {
  return sensors.getDeviceCount();
}

float readTemperature(bool printSerial) {
  sensors.requestTemperatures();
  float currentTemp = sensors.getTempCByIndex(0);

  if (printSerial) {
    Serial.print("1. 1-Wire Bus (Devices): ");
    Serial.println(sensors.getDeviceCount());
    Serial.print("   Temperature read: ");

    if (currentTemp == DEVICE_DISCONNECTED_C) {
      Serial.println("ERROR (-127 C / Disconnected)");
    } else {
      Serial.printf("%.2f °C\n", currentTemp);
    }
  }

  return currentTemp;
}

EnvironmentReadings readAllSensors(bool printSerial) {
  EnvironmentReadings readings;

  readings.temperature = readTemperature(printSerial);
  readings.temperature_valid = (readings.temperature != DEVICE_DISCONNECTED_C);

  MQ135.update();
  MQ135.setRegressionMethod(1);
  readings.voltage_mq135 = MQ135.getVoltage();

  MQ135.setA(110.47); MQ135.setB(-2.862);
  readings.co2 = MQ135.readSensor() + 400.0;

  MQ135.setA(605.18); MQ135.setB(-3.937);
  readings.co_mq135 = MQ135.readSensor();

  MQ135.setA(77.255); MQ135.setB(-3.18);
  readings.alcohol = MQ135.readSensor();

  MQ135.setA(102.2); MQ135.setB(-2.473);
  readings.nh4 = MQ135.readSensor();

  MQ135.setA(44.947); MQ135.setB(-3.445);
  readings.toluene = MQ135.readSensor();

  MQ135.setA(34.668); MQ135.setB(-3.369);
  readings.acetone = MQ135.readSensor();

  MQ4.update();
  MQ4.setRegressionMethod(1);
  readings.voltage_mq4 = MQ4.getVoltage();

  MQ4.setA(1012.7); MQ4.setB(-2.786);
  readings.ch4 = MQ4.readSensor();

  MQ4.setA(3811.9); MQ4.setB(-3.113);
  readings.lpg = MQ4.readSensor();

  MQ4.setA(658.71); MQ4.setB(-2.168);
  readings.propane = MQ4.readSensor();

  MQ4.setA(987.99); MQ4.setB(-2.162);
  readings.h2 = MQ4.readSensor();

  MQ4.setA(30000000); MQ4.setB(-8.308);
  readings.smoke = MQ4.readSensor();

  if (printSerial) {
    Serial.println("\n================ GAS READINGS (PPM) ================");
    Serial.printf("--- [MQ-135 | Pin %d] Voltage: %.2f V ---\n", PIN_MQ135, readings.voltage_mq135);
    Serial.printf("CO2: %.2f ppm | CO: %.2f ppm\n", readings.co2, readings.co_mq135);
    Serial.printf("Alcohol: %.2f ppm | Ammonia: %.2f ppm\n", readings.alcohol, readings.nh4);
    Serial.printf("Toluene: %.2f ppm | Acetone: %.2f ppm\n", readings.toluene, readings.acetone);

    Serial.printf("\n--- [MQ-4 | Pin %d] Voltage: %.2f V ---\n", PIN_MQ4, readings.voltage_mq4);
    Serial.printf("Methane (CH4): %.2f ppm | LPG: %.2f ppm\n", readings.ch4, readings.lpg);
    Serial.printf("Propane: %.2f ppm | Hydrogen: %.2f ppm\n", readings.propane, readings.h2);
    Serial.printf("Smoke: %.2f ppm\n", readings.smoke);
    Serial.println("====================================================\n");
  }

  return readings;
}