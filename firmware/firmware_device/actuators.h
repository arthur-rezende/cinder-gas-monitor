#pragma once
#include <Arduino.h>

void initializeActuators();
void turnOffLights();
void turnOnLight(int lightPin);
void controlFan(bool turnOn);
void setAlarmMuted(bool muted);
bool alarmIsMuted();
bool fanIsOn();
void maintainAlarm();
void startAlarm();
void stopAlarm();
bool button1Pressed();
bool button2Pressed();