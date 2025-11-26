# aplicacao-1-iot

Este projeto usa um ESP32 para controlar uma “lâmpada inteligente” com LED RGB, leitura de sensores e integração com o **Arduino IoT Cloud**.
A ideia é misturar controle físico (potenciômetro, botões, sensores) com controle remoto via dashboard da Arduino Cloud.

---

## 📌 Funcionalidades

* Controle remoto da luz (ligar/desligar)
* Seleção de cores via comandos de texto
* Controle físico por potenciômetro
* Leitura de temperatura (DHT11)
* Leitura de luminosidade (LDR)
* Sistema global ON/OFF usando botão físico
* Alerta por buzzer quando temperatura está fora do normal
* Configuração interativa de Wi-Fi pelo Serial (não precisa deixar senha salva no código)

---

## 🧩 Hardware

| Componente    | Pino                   |
| ------------- | ---------------------- |
| LED RGB       | R = 12, G = 14, B = 27 |
| DHT11         | 21                     |
| LDR           | 34                     |
| Potenciômetro | 32                     |
| Botão         | 5                      |
| Buzzer        | 25                     |

---

## 🔌 Como funciona

### **1. Configuração do Wi-Fi**

Ao iniciar, o ESP32 faz um scan das redes e mostra no Serial.
Você escolhe o número da rede e digita a senha.
Essas informações são salvas nas variáveis `minhaSSID` e `minhaSenha`, e então ele conecta na Arduino Cloud.

### **2. Integração com o Arduino IoT Cloud**

As variáveis principais são:

* `temperatura`
* `luminosidade`
* `luzInteligente` (CloudColoredLight)
* `statusSistema`
* `consoleComando`

O arquivo `thingProperties.h` é o que registra tudo isso.

### **3. Controle da luz**

Existem duas formas:

**a) Comando via Cloud (tem prioridade)**
Ex.: “Vermelho”, “Azul”, “Verde”, “Amarelo”
Fica 1 segundo ligado na cor, e depois volta ao controle normal.

**b) Potenciômetro**
As faixas são convertidas em cores:

* < 1000 → Vermelho
* < 1800 → Amarelo
* < 2600 → Verde
* ≥ 2600 → Azul

### **4. Segurança e sensores**

* Se a temperatura < 0°C ou > 28°C:

  * LED desliga
  * Buzzer toca
  * `statusSistema` muda para perigo

* Se estiver escuro (LDR < limiar) e o detector estiver ativo, o LED acende automaticamente.

### **5. Botão físico**

Alterna entre:

* **Sistema ON**
* **Sistema OFF (tudo desligado)**

---

## 📡 Comandos aceitos pelo dashboard

```
Ligar
Desligar

Vermelho
Amarelo
Verde
Azul

Desativar Temperatura
Ativar Temperatura
Desativar Detector
Ativar Detector
Desativar Buzzer
Ativar Buzzer
```

---

## 🗂️ Estrutura dos arquivos

### **app.ino**

* Lógica principal
* Loop do sistema
* Leitura de sensores
* Controle da luz
* Interrupção do botão
* Segurança por temperatura
* Configuração Wi-Fi interativa

### **thingProperties.h**

* IDs do dispositivo
* Definição das variáveis do Arduino Cloud
* Handlers (`commandChange` e `luzChange`)

---

## ✔️ Como rodar

1. Instale as bibliotecas:

   * ArduinoIoTCloud
   * Arduino_ConnectionHandler
   * DHT sensor library
   * WiFi

2. Faça upload do código no ESP32

3. Abra o Serial Monitor (115200)

4. Escolha a rede e digite a senha

5. Abra o dashboard da Arduino Cloud e teste os comandos

6. Acompanhe temperatura, luminosidade e status no dashboard

