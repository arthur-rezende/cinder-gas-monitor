#include "device_network.h"
#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h>

static const char* AP_SSID = "Cinder-Gas-Monitor";
static const char* AP_PASSWORD = "cinder123";
static const char* HOSTNAME = "cinder-esp32";
static const char* DEFAULT_MQTT_HOST = "192.168.1.10";
static const uint16_t DEFAULT_MQTT_PORT = 1883;
static const int MQTT_HOST_MAX_LEN = 64;
static const int MQTT_PORT_MAX_LEN = 6;

static Preferences preferences;
static WiFiManager wifiManager;
static WiFiManagerParameter mqttHostParam(
  "mqtt_host",
  "MQTT broker IP or hostname",
  DEFAULT_MQTT_HOST,
  MQTT_HOST_MAX_LEN
);
static WiFiManagerParameter mqttPortParam(
  "mqtt_port",
  "MQTT port",
  "1883",
  MQTT_PORT_MAX_LEN
);

static String mqttHost = DEFAULT_MQTT_HOST;
static uint16_t mqttPort = DEFAULT_MQTT_PORT;
static bool paramsRegistered = false;
static bool wasConnected = false;
static unsigned long lastReconnectAttempt = 0;

static String readPreference(const char* key, const char* defaultValue) {
  preferences.begin("cinder", true);
  String value = preferences.getString(key, defaultValue);
  preferences.end();
  return value;
}

static void savePreference(const char* key, const String& value) {
  preferences.begin("cinder", false);
  preferences.putString(key, value);
  preferences.end();
}

static void sanitizeMQTTSettings(const String& hostInput, const String& portInput) {
  mqttHost = hostInput;
  mqttHost.trim();
  if (mqttHost.length() == 0) {
    mqttHost = DEFAULT_MQTT_HOST;
  }

  long parsedPort = portInput.toInt();
  if (parsedPort < 1 || parsedPort > 65535) {
    mqttPort = DEFAULT_MQTT_PORT;
  } else {
    mqttPort = static_cast<uint16_t>(parsedPort);
  }
}

static void syncParameterValues() {
  mqttHostParam.setValue(mqttHost.c_str(), MQTT_HOST_MAX_LEN);
  mqttPortParam.setValue(String(mqttPort).c_str(), MQTT_PORT_MAX_LEN);
}

static void saveParametersCallback() {
  sanitizeMQTTSettings(
    String(mqttHostParam.getValue()),
    String(mqttPortParam.getValue())
  );

  savePreference("mqtt_host", mqttHost);
  savePreference("mqtt_port", String(mqttPort));
  syncParameterValues();
}

static void onPortalStarted(WiFiManager* /*manager*/) {
  Serial.print("Configuration portal started: http://");
  Serial.println(WiFi.softAPIP());
}

bool initializeNetwork() {
  sanitizeMQTTSettings(
    readPreference("mqtt_host", DEFAULT_MQTT_HOST),
    readPreference("mqtt_port", "1883")
  );
  syncParameterValues();

  if (!paramsRegistered) {
    wifiManager.addParameter(&mqttHostParam);
    wifiManager.addParameter(&mqttPortParam);
    paramsRegistered = true;
  }

  wifiManager.setTitle("Cinder-Gas-Monitor");
  wifiManager.setHostname(HOSTNAME);
  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setAPClientCheck(true);
  wifiManager.setConnectTimeout(10);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setSaveParamsCallback(saveParametersCallback);
  wifiManager.setAPCallback(onPortalStarted);

  WiFi.mode(WIFI_STA);
  bool connected = wifiManager.autoConnect(AP_SSID, AP_PASSWORD);
  wasConnected = connected;

  if (connected) {
    Serial.print("Wi-Fi connected: ");
    Serial.println(WiFi.localIP());
  }

  return connected;
}

bool networkConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void maintainNetwork() {
  wifiManager.process();

  bool connected = networkConnected();
  if (connected) {
    if (!wasConnected) {
      Serial.print("Wi-Fi connected: ");
      Serial.println(WiFi.localIP());
    }
    wasConnected = true;
    return;
  }

  wasConnected = false;

  if (wifiManager.getConfigPortalActive()) {
    return;
  }

  unsigned long now = millis();
  if (now - lastReconnectAttempt >= 10000) {
    lastReconnectAttempt = now;
    WiFi.reconnect();
  }
}

void startNetworkConfigurationPortal() {
  if (!wifiManager.getConfigPortalActive()) {
    wifiManager.startConfigPortal(AP_SSID, AP_PASSWORD);
  }
}

void resetAndStartNetworkConfigurationPortal() {
  Serial.println("Resetting Wi-Fi settings and starting configuration portal...");
  if (wifiManager.getConfigPortalActive()) {
    wifiManager.stopConfigPortal();
  }
  wifiManager.resetSettings();
  wifiManager.startConfigPortal(AP_SSID, AP_PASSWORD);
}

String getMQTTHost() {
  return mqttHost;
}

uint16_t getMQTTPort() {
  return mqttPort;
}
