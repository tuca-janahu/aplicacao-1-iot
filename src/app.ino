#include "thingProperties.h"
#include <DHT.h>
#include <WiFi.h> 

#define POT        32  
#define LDR        34  
#define BOTAO      5
#define LED_R      12
#define LED_G      14
#define LED_B      27
#define BUZZER     25
#define DHT        21

#define DHTTYPE DHT11
DHT dht(DHT, DHTTYPE);

volatile bool sistemaFisicoAtivo = true; 
const int LIMIAR_LDR = 1300; 
unsigned long lastTime = 0;

// Controle de Cor via Comando (Prioridade sobre Potenciômetro)
unsigned long commandTime = 0;
bool commandColor = false;
int r_cmd, g_cmd, b_cmd;

bool isTemperatureOn = true;
bool isLightOn = true;
bool isBuzzerOn = true;

// --- circuito físico ---
void defineColor(int r, int g, int b) {
  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
}

void defineColorPot(int valor) {
  if (valor < 1000) defineColor(255, 0, 0);       // Vermelho
  else if (valor < 1800) defineColor(255, 200, 0);  // Amarelo
  else if (valor < 2600) defineColor(0, 255, 0);  // Verde
  else defineColor(0, 0, 255);                    // Azul
}

// 
void setup_wifi_interactive() {
    delay(10);
    Serial.println("\n--- INICIANDO CONFIGURAÇÃO WI-FI ---");
    Serial.println("Procurando redes Wi-Fi...");
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    int n = WiFi.scanNetworks();
    
    if (n == 0) {
        Serial.println("Nenhuma rede encontrada. O ESP32 será reiniciado.");
        delay(2000);
        ESP.restart();
    } else {
        Serial.print(n); Serial.println(" redes encontradas:");
        for (int i = 0; i < n; ++i) {
            Serial.print(i + 1); Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (Sinal: "); Serial.print(WiFi.RSSI(i)); Serial.println(")");
            delay(10);
        }
    }

    Serial.println("\nDigite o NÚMERO da rede desejada:");
    while (Serial.available() == 0) {} // Aguarda input
    
    String network_choice_str = Serial.readStringUntil('\n');
    int network_index = network_choice_str.toInt() - 1;

    if (network_index < 0 || network_index >= n) {
        Serial.println("Opção inválida. Reiniciando...");
        delay(1000); ESP.restart();
    }
    
    // Salva na variável global (minhaSSID) definida no thingProperties.h
    minhaSSID = WiFi.SSID(network_index);

    Serial.print("Rede selecionada: "); Serial.println(minhaSSID);
    Serial.println("Digite a SENHA da rede:");
    
    while (Serial.available() == 0) {} // Aguarda input
    
    // Salva na variável global (minhaSenha)
    minhaSenha = Serial.readStringUntil('\n');
    minhaSenha.trim(); // Remove espaços e quebras de linha extras

    Serial.println("Credenciais recebidas! Conectando à Arduino Cloud...");
}

// função para parar o sistema
void IRAM_ATTR stopButton() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > 200) { 
    sistemaFisicoAtivo = !sistemaFisicoAtivo;
    lastInterruptTime = interruptTime;
  }
}

// conexão com cloud 
void luzChange() {
  if (luzInteligente.getSwitch()) {
    Serial.println("Cloud: Ligando...");
  } else {
    Serial.println("Cloud: Desligando...");
    defineColor(0,0,0);
  }
}

void commandChange() {
  String cmd = consoleComando;
  Serial.println("Comando: " + cmd);

  if (cmd.equalsIgnoreCase("Ligar")) { luzInteligente.setSwitch(true); }
  else if (cmd.equalsIgnoreCase("Desligar")) { luzInteligente.setSwitch(false); }
  
  else if (cmd.equalsIgnoreCase("Desativar Temperatura")) { isTemperatureOn = false; }
  else if (cmd.equalsIgnoreCase("Ativar Temperatura")) { isTemperatureOn = true; }
  
  else if (cmd.equalsIgnoreCase("Desativar Detector")) { isLightOn = false; }
  else if (cmd.equalsIgnoreCase("Ativar Detector")) { isLightOn = true; }
  
  else if (cmd.equalsIgnoreCase("Desativar Buzzer")) { isBuzzerOn = false; }
  else if (cmd.equalsIgnoreCase("Ativar Buzzer")) { isBuzzerOn = true; }
  
  // LUZ RGB
  else if (cmd.equalsIgnoreCase("Vermelho")) { 
    r_cmd=255; g_cmd=0; b_cmd=0; commandColor=true; commandTime=millis(); luzInteligente.setSwitch(true);
  }
  else if (cmd.equalsIgnoreCase("Amarelo")) { 
    r_cmd=255; g_cmd=200; b_cmd=0; commandColor=true; commandTime=millis(); luzInteligente.setSwitch(true);
  }
  else if (cmd.equalsIgnoreCase("Verde")) { 
    r_cmd=0; g_cmd=255; b_cmd=0; commandColor=true; commandTime=millis(); luzInteligente.setSwitch(true);
  }
  else if (cmd.equalsIgnoreCase("Azul")) { 
    r_cmd=0; g_cmd=0; b_cmd=255; commandColor=true; commandTime=millis(); luzInteligente.setSwitch(true);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  setup_wifi_interactive();
  initProperties();
  ArduinoIoTPreferredConnection = new WiFiConnectionHandler(minhaSSID.c_str(), minhaSenha.c_str());
  ArduinoCloud.begin(*ArduinoIoTPreferredConnection);
  
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // PINAGEM
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BOTAO, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(BOTAO), stopButton, FALLING);
  dht.begin();
}

void loop() {
  ArduinoCloud.update(); 

  // Reset da cor inserida
  if (commandColor && millis() - commandTime > 1000) {
    commandColor = false;
    if (!luzInteligente.getSwitch()) defineColor(0,0,0);
  }

  if (millis() - lastTime > 2000) { 
    lastTime = millis();
    
    float t = dht.readTemperature();
    int l = analogRead(LDR);
    int p = analogRead(POT);

    if (!isnan(t)) temperatura = t;
    luminosidade = l;

    Serial.print("T: "); Serial.print(t); Serial.print("C | L: "); Serial.println(l);

    // verificação de botao fisico
    if (!sistemaFisicoAtivo) {
       statusSistema = "SISTEMA OFF (BTN)";
       defineColor(0, 0, 0);
       digitalWrite(BUZZER, LOW);
       return; 
    }

    // limites de temperatura
    bool perigo = (temperatura < 0 || temperatura > 28);
    
    if (perigo && isTemperatureOn) {
      statusSistema = "PERIGO! TEMP"; 
      Serial.println("Perigo! Desligar!"); //
      defineColor(0, 0, 0); 
      
      if (isBuzzerOn) digitalWrite(BUZZER, HIGH); 
      if (luzInteligente.getSwitch()) luzInteligente.setSwitch(false); 
    } 
    else {
      digitalWrite(BUZZER, LOW);
      statusSistema = "NORMAL";
      
      bool isCloudOn = luzInteligente.getSwitch();
      bool lowLight = (luminosidade < LIMIAR_LDR);

      if (isCloudOn || (lowLight && isLightOn)) {
          
          if (commandColor) {
            // comando de texto define a cor
            defineColor(r_cmd, g_cmd, b_cmd);
          } else {
            // potenciometro define a cor
            defineColorPot(p); 
          }
          
          if (!luzInteligente.getSwitch()) luzInteligente.setSwitch(true);

      } else {
          // Desligado
          defineColor(0, 0, 0);
          if (luzInteligente.getSwitch()) luzInteligente.setSwitch(false);
      }
    }
  }
}

