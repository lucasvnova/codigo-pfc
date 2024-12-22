#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <BH1750.h>
#include <RTClib.h>

// Definindo os objetos para os sensores
Adafruit_TCS34725 tcs = Adafruit_TCS34725();
BH1750 lightMeter;
RTC_DS3231 rtc;

// Pinos do LED RGB
int ledPinR = 9;  // Pino do LED RGB Vermelho
int ledPinG = 10; // Pino do LED RGB Verde
int ledPinB = 11; // Pino do LED RGB Azul

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  // Inicializando o sensor TCS34725
  if (tcs.begin()) {
    Serial.println("Sensor TCS34725 encontrado!");
  } else {
    Serial.println("Não foi possível encontrar o sensor TCS34725");
    while (1);
  }

  // Inicializando o sensor de iluminância BH1750
  lightMeter.begin();

  // Configurando os pinos como saída
  pinMode(ledPinR, OUTPUT);
  pinMode(ledPinG, OUTPUT);
  pinMode(ledPinB, OUTPUT);

// Verifica se o RTC está conectado
  if (!rtc.begin()) {
    Serial.println("Erro: RTC não encontrado. Verifique a conexão.");
    while (1); // Interrompe a execução
  }
  
  // Verifica se o RTC está configurado corretamente
  if (rtc.lostPower()) {
    Serial.println("O RTC perdeu a alimentação. Configurando a data e hora...");
    // Configura o RTC para a data e hora do computador (somente na primeira vez)
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

}

// Função para calcular a temperatura de cor baseada na hora atual
int getTargetColorTemp(int hour) {
  if (hour >= 6 && hour < 9) {
    return 6500;  // Manhã (Fria)
  } else if (hour >= 9 && hour < 12) {
    return 5000;  // Meio-dia (Fria)
  } else if (hour >= 12 && hour < 17) {
    return 4000;  // Tarde (Neutra)
  } else if (hour >= 17 && hour < 18) {
    return 3500;  // Final de tarde (Quente)
  } else if (hour >= 18 && hour < 21) {
    return 2800;  // Noite (Quente)
  } else {
    return 2700;  // Madrugada (Quente)
  }
}

// Função para ler a temperatura de cor do sensor TCS34725
int getColorTemp() {
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
  int colorTemp = tcs.calculateColorTemperature(r, g, b);
  return colorTemp;
}

// Função para ajustar a cor do LED RGB com base na temperatura de cor
void adjustLEDColor(int targetTemp, int currentTemp, float intensityFactor) {
  int diff = targetTemp - currentTemp;
  int red, green, blue;

    // Ajustes proporcionais com base no valor de diff
  if (diff <= -1000) {
    // Muito acima da ideal -> Vermelho intenso
    red = 255;
    green = 30;
    blue = 30;
  } else if (diff > -1000 && diff <= -800) {
    red = 255;
    green = 80;
    blue = 60;
  } else if (diff > -800 && diff <= -600) {
    red = 255;
    green = 120;
    blue = 90;
  } else if (diff > -600 && diff <= -400) {
    red = 255;
    green = 160;
    blue = 120;
  } else if (diff > -400 && diff <= -200) {
    red = 255;
    green = 200;
    blue = 150;
  } else if (diff > -200 && diff < 200) {
    // Próximo da ideal -> Branco equilibrado
    red = 220;
    green = 220;
    blue = 200;
  } else if (diff >= 200 && diff < 400) {
    red = 200;
    green = 220;
    blue = 255;
  } else if (diff >= 400 && diff < 600) {
    red = 160;
    green = 200;
    blue = 255;
  } else if (diff >= 600 && diff < 800) {
    red = 120;
    green = 180;
    blue = 255;
  } else if (diff >= 800 && diff < 1000) {
    red = 80;
    green = 160;
    blue = 255;
  } else if (diff >= 1000) {
    // Muito abaixo da ideal -> Azul intenso
    red = 30;
    green = 50;
    blue = 255;
  }

  // Aplicando o fator de intensidade
  red = constrain(red * intensityFactor, 0, 255);
  green = constrain(green * intensityFactor, 0, 255);
  blue = constrain(blue * intensityFactor, 0, 255);

  analogWrite(ledPinR, red);
  analogWrite(ledPinG, green);
  analogWrite(ledPinB, blue);
}

// Função revisada para calcular o fator de intensidade baseado na iluminância
float calculateIntensityFactor(float lux) {
  float maxLux = 500; // Valor máximo de referência
  return constrain((maxLux - lux) / maxLux, 0, 1); // Calcula o fator de 0 a 1
}

void loop() {
  
  // Obtendo a hora atual
  DateTime now = rtc.now();
  int hour = now.hour(); // Obtendo a hora atual do RTC

  // Obtendo a iluminância
  float lux = lightMeter.readLightLevel();

  // Calculando o fator de intensidade
  float intensityFactor = calculateIntensityFactor(lux);

  // Obtendo a temperatura de cor alvo
  int targetTemp = getTargetColorTemp(hour);

  // Lendo a temperatura de cor atual
  int currentTemp = getColorTemp();

  // Ajustando a cor do LED
  adjustLEDColor(targetTemp, currentTemp, intensityFactor);

  // Monitor serial para depuração
  Serial.print("Hora: ");
  Serial.print(hour);
  Serial.print("h  |  ");
  Serial.print("Temperatura de Cor Alvo: ");
  Serial.print(targetTemp);
  Serial.print("K  |  ");
  Serial.print("Temperatura de Cor Atual: ");
  Serial.print(currentTemp);
  Serial.print("K  |  ");
  Serial.print("Iluminância: ");
  Serial.print(lux);
  Serial.print(" lux  |  ");
  Serial.print("Fator de Intensidade: ");
  Serial.println(intensityFactor);

  // Aguarda um tempo antes de atualizar
  delay(1000);
}