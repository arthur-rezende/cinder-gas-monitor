#include <OneWire.h>
#include <DallasTemperature.h>
#include <MQUnifiedsensor.h>

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
const int red      = 18;
const int green    = 19;
const int blue     = 21;
const int buzzer   = 15;
const int pinoRele = 25; // Controle do ventilador 5V

const int btn_1    = 16; // Aciona o Modo de Teste (Pull-down externo)
const int btn_2    = 17; // Silencia a música (Pull-down externo)

// ======== Setup do Sensor de Temperatura (DS18B20) =========
const int oneWireBus = 4;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// ======== Setup dos Sensores de Gás =========
#define placa "ESP-32"
#define Voltage_Resolution 3.3
#define ADC_Bit_Resolution 12

const int pinMQ4   = 34; // ADC1_CH6
const int pinMQ135 = 35; // ADC1_CH7

// Fatores de ar limpo
#define RatioMQ135CleanAir 3.6
#define RatioMQ4CleanAir   4.4

MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pinMQ135, "MQ-135");
MQUnifiedsensor MQ4(placa, Voltage_Resolution, ADC_Bit_Resolution, pinMQ4, "MQ-4");

// ======== Estrutura de Dados dos Gases =========
struct LeiturasGases {
  // MQ-135
  float co2;
  float co_mq135;
  float alcool;
  float nh4;
  float tolueno;
  float acetona;

  // MQ-4
  float ch4;
  float glp;
  float propano;
  float h2;
  float fumaca;
};

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

// ======== Protótipos das Funções ========
void apagarLuzes();
void acenderLuz(const int luz);
void controlarVentilador(bool ligar);
void calibrarSensoresMQ();
LeiturasGases lerTodosOsGases();
void tocarMusicaTeste();
void debugMode();

void setup() {
  Serial.begin(115200);
  delay(500);

  // Inicializa DS18B20
  sensors.begin();

  // Configura pinos de saída
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(pinoRele, OUTPUT);

  // Garante estado inicial desligado
  apagarLuzes();
  controlarVentilador(false);

  // Pull-down físico externo (solto = LOW, pressionado = HIGH)
  pinMode(btn_1, INPUT);
  pinMode(btn_2, INPUT);

  // Inicializa e calibra os sensores MQ
  calibrarSensoresMQ();

  // Executa rotina de diagnóstico ao iniciar
  debugMode();
}

void loop() {
  // Aperte btn_1 para rodar o teste novamente
  if (digitalRead(btn_1) == HIGH) {
    delay(250);
    debugMode();
  }
}

// ======== Configuração e Calibração dos Sensores MQ ========
void calibrarSensoresMQ() {
  Serial.println("Calibrando sensores MQ em ar limpo...");

  // Configuração MQ-135 (Pino 35)
  MQ135.setRegressionMethod(1); // _PPM = a * ratio^b
  MQ135.setA(110.47);
  MQ135.setB(-2.862);
  MQ135.init();

  float calcR0_135 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ135.update();
    calcR0_135 += MQ135.calibrate(RatioMQ135CleanAir);
  }
  MQ135.setR0(calcR0_135 / 10.0);

  // Configuração MQ-4 (Pino 34)
  MQ4.setRegressionMethod(1);
  MQ4.setA(1012.7);
  MQ4.setB(-2.786);
  MQ4.init();

  float calcR0_4 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ4.update();
    calcR0_4 += MQ4.calibrate(RatioMQ4CleanAir);
  }
  MQ4.setR0(calcR0_4 / 10.0);

  if (isinf(calcR0_135) || isinf(calcR0_4)) {
    Serial.println("[AVISO] R0 infinito detectado! Verifique se os pinos 34 e 35 estao conectados.");
  } else {
    Serial.println("Calibracao MQ concluida!");
  }
}

// ======== Leitura Detalhada de Todos os Gases (MQ-135 e MQ-4) ========
LeiturasGases lerTodosOsGases() {
  LeiturasGases dados;

  Serial.println("\n================ LEITURA DE GASES (PPM) ================");

  // 1. SENSOR MQ-135 (Pino 35 - Qualidade do Ar)
  MQ135.update();
  MQ135.setRegressionMethod(1);

  MQ135.setA(110.47); MQ135.setB(-2.862);
  dados.co2 = MQ135.readSensor() + 400.0; // +400 ppm base atmosférica

  MQ135.setA(605.18); MQ135.setB(-3.937);
  dados.co_mq135 = MQ135.readSensor();

  MQ135.setA(77.255); MQ135.setB(-3.18);
  dados.alcool = MQ135.readSensor();

  MQ135.setA(102.2); MQ135.setB(-2.473);
  dados.nh4 = MQ135.readSensor();

  MQ135.setA(44.947); MQ135.setB(-3.445);
  dados.tolueno = MQ135.readSensor();

  MQ135.setA(34.668); MQ135.setB(-3.369);
  dados.acetona = MQ135.readSensor();

  Serial.printf("--- [MQ-135 | Pino %d] Tensao: %.2f V ---\n", pinMQ135, MQ135.getVoltage());
  Serial.printf("CO2:     %.2f ppm\n", dados.co2);
  Serial.printf("CO:      %.2f ppm\n", dados.co_mq135);
  Serial.printf("Alcool:  %.2f ppm\n", dados.alcool);
  Serial.printf("Amonia:  %.2f ppm\n", dados.nh4);
  Serial.printf("Tolueno: %.2f ppm\n", dados.tolueno);
  Serial.printf("Acetona: %.2f ppm\n", dados.acetona);

  // 2. SENSOR MQ-4 (Pino 34 - Gases Combustíveis / Metano)
  MQ4.update();
  MQ4.setRegressionMethod(1);

  MQ4.setA(1012.7); MQ4.setB(-2.786);
  dados.ch4 = MQ4.readSensor();

  MQ4.setA(3811.9); MQ4.setB(-3.113);
  dados.glp = MQ4.readSensor();

  MQ4.setA(658.71); MQ4.setB(-2.168);
  dados.propano = MQ4.readSensor();

  MQ4.setA(987.99); MQ4.setB(-2.162);
  dados.h2 = MQ4.readSensor();

  MQ4.setA(30000000); MQ4.setB(-8.308);
  dados.fumaca = MQ4.readSensor();

  Serial.printf("\n--- [MQ-4 | Pino %d] Tensao: %.2f V ---\n", pinMQ4, MQ4.getVoltage());
  Serial.printf("Metano (CH4): %.2f ppm\n", dados.ch4);
  Serial.printf("GLP:          %.2f ppm\n", dados.glp);
  Serial.printf("Propano:      %.2f ppm\n", dados.propano);
  Serial.printf("Hidrogenio:   %.2f ppm\n", dados.h2);
  Serial.printf("Fumaca:       %.2f ppm\n", dados.fumaca);
  Serial.println("========================================================\n");

  return dados;
}

// ======== Funções Auxiliares de Atuadores ========
void apagarLuzes() {
  // LED RGB Ânodo Comum: HIGH apaga
  digitalWrite(red, HIGH);
  digitalWrite(green, HIGH);
  digitalWrite(blue, HIGH);
}

void acenderLuz(const int luz) {
  apagarLuzes();
  // LED RGB Ânodo Comum: LOW acende
  digitalWrite(luz, LOW);
}

void controlarVentilador(bool ligar) {
  // Acionar módulo relé
  if (ligar) {
    digitalWrite(pinoRele, LOW);
  } else {
    digitalWrite(pinoRele, HIGH);
  }
}

// ======== Função da Música ========
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

  // 1. Teste do Sensor de Temperatura (DS18B20)
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

  // 2. Teste Completo dos Sensores de Gás (MQ-135 e MQ-4)
  Serial.println("2. Testando Sensores de Gas (MQ)...");
  lerTodosOsGases();

  // 3. Teste Individual dos LEDs (Ânodo Comum)
  Serial.println("3. Testando LEDs...");
  
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

  // 4. Teste do Relé / Ventilador 5V
  Serial.println("4. Testando Rele / Ventilador (Pino 25) por 2s...");
  controlarVentilador(true);
  delay(2000);
  controlarVentilador(false);
  Serial.println("   [Ventilador desligado]");

  // 5. Teste Sonoro com Asa Branca
  tocarMusicaTeste();

  // 6. Estado dos Botões (com pull-down: LOW = SOLTO, HIGH = PRESSIONADO)
  Serial.print("5. Estado Botao 1 (Pino 16): ");
  Serial.println(digitalRead(btn_1) == LOW ? "SOLTO (OK)" : "PRESSIONADO");
  Serial.print("   Estado Botao 2 (Pino 17): ");
  Serial.println(digitalRead(btn_2) == LOW ? "SOLTO (OK)" : "PRESSIONADO");

  Serial.println("===== FIM DO TESTE =====\n");
}