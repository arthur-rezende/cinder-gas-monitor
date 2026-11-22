#pragma once
#include <Arduino.h>

// ======== Pinos de Saída (Atuadores) =========
const int PIN_RED    = 21;
const int PIN_GREEN  = 19;
const int PIN_BLUE   = 18;
const int PIN_BUZZER = 15;
const int PIN_RELE   = 25;

// ======== Pinos de Entrada =========
const int PIN_BTN_1  = 16; // Aciona o Modo de Teste
const int PIN_BTN_2  = 17; // Silencia a música

// ======== Pinos de Sensores =========
const int PIN_ONE_WIRE = 4;  // DS18B20
const int PIN_MQ4      = 34; // Gases Combustíveis
const int PIN_MQ135    = 35; // Qualidade do Ar

// ======== Parâmetros de Hardware ADC =========
#define BOARD_NAME "ESP-32"
#define VOLTAGE_RESOLUTION 3.3
#define ADC_BIT_RESOLUTION 12

// Fatores de ar limpo
#define RATIO_MQ135_CLEAN_AIR 3.6
#define RATIO_MQ4_CLEAN_AIR   4.4

// ======== Limiares de Alerta e Perigo =========
const float TEMP_WARNING = 30.0;
const float TEMP_DANGER = 40.0;

const float CO2_WARNING = 800.0;
const float CO2_DANGER = 1500.0;

const float CH4_WARNING = 200.0;
const float CH4_DANGER = 800.0;

const float SMOKE_WARNING = 100.0;
const float SMOKE_DANGER = 500.0;

const unsigned long ALARM_MUTE_MS = 60000;
const unsigned long SENSOR_INTERVAL_MS = 5000;
const unsigned long BUTTON_DEBOUNCE_MS = 50;
const unsigned long MQTT_ACTUATOR_INTERVAL_MS = 5000;