#pragma once
#include <Arduino.h>

bool initializeNetwork();
bool networkConnected();
void maintainNetwork();
void startNetworkConfigurationPortal();
void resetAndStartNetworkConfigurationPortal();

String getMQTTHost();
uint16_t getMQTTPort();