#include "actuators.h"
#include "config.h"

static bool alarmMuted = false;
static bool alarmActive = false;
static bool buzzerOn = false;
static bool fanOn = false;
static unsigned long lastBuzzerChange = 0;

void initializeActuators() {
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_RELE, OUTPUT);

  // Pull-down externo
  pinMode(PIN_BTN_1, INPUT);
  pinMode(PIN_BTN_2, INPUT);

  turnOffLights();
  controlFan(false);
  stopAlarm();
}

void turnOffLights() {
  // LED RGB Ânodo Comum: HIGH desliga
  digitalWrite(PIN_RED, HIGH);
  digitalWrite(PIN_GREEN, HIGH);
  digitalWrite(PIN_BLUE, HIGH);
}

void turnOnLight(int lightPin) {
  turnOffLights();
  // LED RGB Ânodo Comum: LOW acende
  digitalWrite(lightPin, LOW);
}

void controlFan(bool turnOn) {
  fanOn = turnOn;
  // Módulo relé ativo em LOW
  digitalWrite(PIN_RELE, turnOn ? LOW : HIGH);
}

void setAlarmMuted(bool muted) {
  alarmMuted = muted;

  if (muted) {
    noTone(PIN_BUZZER);
    buzzerOn = false;
  }
}

bool alarmIsMuted() {
  return alarmMuted;
}

void startAlarm() {
  alarmActive = true;
}

void stopAlarm() {
  alarmActive = false;
  noTone(PIN_BUZZER);
  buzzerOn = false;
}

void maintainAlarm() {
  if (!alarmActive || alarmMuted) {
    if (buzzerOn) {
      noTone(PIN_BUZZER);
      buzzerOn = false;
    }
    return;
  }

  unsigned long now = millis();
  if (now - lastBuzzerChange >= (buzzerOn ? 250 : 500)) {
    lastBuzzerChange = now;

    if (buzzerOn) {
      noTone(PIN_BUZZER);
      buzzerOn = false;
    } else {
      tone(PIN_BUZZER, 1200);
      buzzerOn = true;
    }
  }
}

bool button1Pressed() {
  return digitalRead(PIN_BTN_1) == HIGH;
}

bool button2Pressed() {
  return digitalRead(PIN_BTN_2) == HIGH;
}

bool fanIsOn() {
  return fanOn;
}