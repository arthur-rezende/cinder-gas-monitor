#include <OneWire.h>
#include <DallasTemperature.h>

// ======== Definições das Notas Musicais ========
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0

// ======== Configurações de Pino (ESP32) =========
const int red    = 18;
const int green  = 19;
const int blue   = 21;
const int buzzer = 15;

const int btn_1  = 16; // Aciona o Modo de Teste (Pull-down externo)
const int btn_2  = 17; // Silencia a música (Pull-down externo)

// ======== Setup do Sensor de Temperatura =========
const int oneWireBus = 4;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// ======== Configurações da Música (Asa Branca) =========
const int tempo = 120;
const int wholenote = (60000 * 4) / tempo;

const int melody[] = {
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

const int totalNotas = sizeof(melody) / sizeof(melody[0]) / 2;

void setup() {
  Serial.begin(115200);
  delay(500);

  sensors.begin();

  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // Pull-down físico externo (solto = LOW, pressionado = HIGH)
  pinMode(btn_1, INPUT_PULLDOWN);
  pinMode(btn_2, INPUT_PULLDOWN);

  apagarLuzes();
  debugMode();
}

void loop() {
  // Aperte btn_1 para rodar o teste novamente
  if (digitalRead(btn_1) == HIGH) {
    delay(250);
    debugMode();
  }
}

// ======== Funções Auxiliares (LED RGB Ânodo Comum) ========

void apagarLuzes() {
  digitalWrite(red, HIGH);
  digitalWrite(green, HIGH);
  digitalWrite(blue, HIGH);
}

void acenderLuz(const int luz) {
  apagarLuzes();
  digitalWrite(luz, LOW);
}

// ======== Função da Música (Asa Branca com Mute Instantâneo) ========

void tocarMusicaTeste() {
  Serial.println("-> Testando Buzzer (Asa Branca)... [Pressione btn_2 para silenciar]");

  for (int thisNote = 0; thisNote < totalNotas * 2; thisNote += 2) {
    int divider = melody[thisNote + 1];
    int noteDuration = 0;

    if (divider > 0) {
      noteDuration = wholenote / divider;
    } else if (divider < 0) {
      noteDuration = (wholenote / abs(divider)) * 1.5;
    }

    int frequencia = melody[thisNote];
    if (frequencia != REST) {
      tone(buzzer, frequencia, noteDuration * 0.9);
    }

    // Espera o tempo da nota monitorando o btn_2 em tempo real
    unsigned long inicioNota = millis();
    while (millis() - inicioNota < (unsigned long)noteDuration) {
      if (digitalRead(btn_2) == HIGH) {
        noTone(buzzer);
        Serial.println("   [Teste de audio interrompido pelo btn_2]");
        return;
      }
    }

    noTone(buzzer);
  }

  Serial.println("   [Teste de audio concluido]");
}

// ======== Rotina Principal de Diagnóstico ========

void debugMode() {
  Serial.println("\n===== MODO DE TESTE DE HARDWARE =====");

  // 1. Teste do Sensor DS18B20
  Serial.print("1. Barramento 1-Wire (Dispositivos): ");
  Serial.println(sensors.getDeviceCount());

  sensors.requestTemperatures();
  float tempAtual = sensors.getTempCByIndex(0);
  Serial.print("   Temperatura lida: ");
  if (tempAtual == DEVICE_DISCONNECTED_C) {
    Serial.println("ERRO (-127 C / Desconectado)");
  } else {
    Serial.print(tempAtual);
    Serial.println(" °C");
  }

  // 2. Teste Individual dos LEDs (Ânodo Comum)
  Serial.println("2. Testando LEDs...");
  
  Serial.println("   [VERDE - Pino 19]");
  acenderLuz(green);
  delay(1000);

  Serial.println("   [AZUL - Pino 21]");
  acenderLuz(blue);
  delay(1000);

  Serial.println("   [VERMELHO - Pino 18]");
  acenderLuz(red);
  delay(1000);

  apagarLuzes();
  Serial.println("   [TODOS APAGADOS]");

  // 3. Teste Sonoro com Asa Branca
  tocarMusicaTeste();

  // 4. Estado dos Botões (com pull-down: LOW = SOLTO, HIGH = PRESSIONADO)
  Serial.print("3. Estado Botao 1 (Pino 16): ");
  Serial.println(digitalRead(btn_1) == LOW ? "SOLTO (OK)" : "PRESSIONADO");
  Serial.print("   Estado Botao 2 (Pino 17): ");
  Serial.println(digitalRead(btn_2) == LOW ? "SOLTO (OK)" : "PRESSIONADO");

  Serial.println("===== FIM DO TESTE =====\n");
}