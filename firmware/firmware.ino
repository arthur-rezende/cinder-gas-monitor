#include <OneWire.h>
#include <DallasTemperature.h>

// ======== Configurações de Pino (ESP32) =========
const int red    = 18;
const int green  = 19;
const int blue   = 21;
const int buzzer = 15;

const int btn_1  = 16; // Aciona o Modo de Teste
const int btn_2  = 17; // Botão auxiliar de teste

// ======== Setup do Sensor de Temperatura =========
const int oneWireBus = 4;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  delay(500); // Aguarda estabilização da Serial

  sensors.begin();

  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // Usa resistor interno do ESP32 (ligue os botões entre o pino e o GND)
  pinMode(btn_1, INPUT_PULLUP);
  pinMode(btn_2, INPUT_PULLUP);

  apagarLuzes();
  debugMode();
}

void loop() {
  debugMode();
}

// ======== Funções Auxiliares ========

void apagarLuzes() {
  digitalWrite(red, HIGH);
  digitalWrite(green, HIGH);
  digitalWrite(blue, HIGH);
}

void acenderLuz(const int luz) {
  apagarLuzes();
  digitalWrite(luz, LOW);
}

void alertaSonoroTeste() {
  Serial.println("-> Testando Buzzer (2 bips curtos)...");
  for (int i = 0; i < 2; i++) {
    tone(buzzer, 2000); // 2000 Hz (frequência limpa para qualquer buzzer)
    delay(150);
    noTone(buzzer);
    delay(150);
  }
}

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

  // 2. Teste Individual dos LEDs
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

  // 3. Teste do Buzzer
  alertaSonoroTeste();

  // 4. Estado dos Botões (em repouso devem ler 1 / HIGH)
  Serial.print("3. Estado Botao 1 (Pino 16): ");
  Serial.println(digitalRead(btn_1) == HIGH ? "SOLTO (OK)" : "PRESSIONADO");
  Serial.print("   Estado Botao 2 (Pino 17): ");
  Serial.println(digitalRead(btn_2) == HIGH ? "SOLTO (OK)" : "PRESSIONADO");

  Serial.println("===== FIM DO TESTE =====\n");
}