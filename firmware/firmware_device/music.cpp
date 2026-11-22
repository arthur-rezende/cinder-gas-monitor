#include "music.h"
#include "config.h"
#include "notes.h"
#include "actuators.h"

static const int tempo = 120;
static const int wholeNote = (60000 * 4) / tempo;

static const int melody[] = {
  NOTE_G4,8, NOTE_A4,8, NOTE_B4,4, NOTE_D5,4, NOTE_D5,4, NOTE_B4,4,
  NOTE_C5,4, NOTE_C5,2, NOTE_G4,8, NOTE_A4,8,
  NOTE_B4,4, NOTE_D5,4, NOTE_D5,4, NOTE_C5,4,

  NOTE_B4,2, REST,8, NOTE_G4,8, NOTE_G4,8, NOTE_A4,8,
  NOTE_B4,4, NOTE_D5,4, REST,8, NOTE_D5,8, NOTE_C5,8, NOTE_B4,8,
  NOTE_G4,4, NOTE_C5,4, REST,8, NOTE_C5,8, NOTE_B4,8, NOTE_A4,8,

  NOTE_A4,4, NOTE_B4,4, REST,8, NOTE_B4,8, NOTE_A4,8, NOTE_G4,8,
  NOTE_G4,2, REST,8, NOTE_G4,8, NOTE_G4,8, NOTE_A4,8,
  NOTE_B4,4, NOTE_D5,4, REST,8, NOTE_D5,8, NOTE_C5,8, NOTE_B4,8,

  NOTE_G4,4, NOTE_C5,4, REST,8, NOTE_C5,8, NOTE_B4,8, NOTE_A4,8,
  NOTE_A4,4, NOTE_B4,4, REST,8, NOTE_B4,8, NOTE_A4,8, NOTE_G4,8,
  NOTE_G4,4, NOTE_F5,8, NOTE_D5,8, NOTE_E5,8, NOTE_C5,8, NOTE_D5,8, NOTE_B4,8,

  NOTE_C5,8, NOTE_A4,8, NOTE_B4,8, NOTE_G4,8, NOTE_A4,8, NOTE_G4,8, NOTE_E4,8, NOTE_G4,8,
  NOTE_G4,4, NOTE_F5,8, NOTE_D5,8, NOTE_E5,8, NOTE_C5,8, NOTE_D5,8, NOTE_B4,8,
  NOTE_C5,8, NOTE_A4,8, NOTE_B4,8, NOTE_G4,8, NOTE_A4,8, NOTE_G4,8, NOTE_E4,8, NOTE_G4,8,
  NOTE_G4,-2, REST,4
};

static const int totalNotes = sizeof(melody) / sizeof(melody[0]) / 2;
static int currentNote = 0;
static unsigned long noteEndsAt = 0;
static bool musicActive = false;

void playTestMusic() {
  Serial.println("-> Testing Buzzer (Asa Branca)... [Press btn_2 to mute]");
  currentNote = 0;
  noteEndsAt = millis();
  musicActive = true;
}

void maintainMusic() {
  if (!musicActive) {
    return;
  }

  if (button2Pressed()) {
    noTone(PIN_BUZZER);
    musicActive = false;
    Serial.println("   [Audio test interrupted by btn_2]");
    return;
  }

  unsigned long now = millis();
  if (now < noteEndsAt) {
    return;
  }

  noTone(PIN_BUZZER);

  if (currentNote >= totalNotes * 2) {
    musicActive = false;
    Serial.println("   [Audio test completed]");
    return;
  }

  int divider = melody[currentNote + 1];
  int noteDuration = divider > 0
    ? (wholeNote / divider)
    : (wholeNote / abs(divider)) * 1.5;

  int frequency = melody[currentNote];
  if (frequency != REST) {
    tone(PIN_BUZZER, frequency, noteDuration * 0.9);
  }

  noteEndsAt = now + noteDuration;
  currentNote += 2;
}