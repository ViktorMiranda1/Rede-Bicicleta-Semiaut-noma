#include <esp_now.h>
#include <WiFi.h>

// MODOS: Descomente APENAS UMA das linhas abaixo para escolher a placa
#define MODO_TRANSMISSOR 
//#define MODO_RECEPTOR_DIREITO
//#define MODO_RECEPTOR_ESQUERDO

// Configurações de hardware comuns
const int pinoPWM = 18;
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Estrutura de dados comum
typedef struct struct_message {
    int valor;
    int duracao;
} struct_message;

struct_message data;

// Variáveis exclusivas do Transmissor
#ifdef MODO_TRANSMISSOR
  bool ackRecebidoR = false;
  bool ackRecebidoL = false;
  bool erroConexao = false;

  unsigned long tempoUltimoComando = 0;
  unsigned long tempoUltimoEnvio = 0;
  const unsigned long TIMEOUT_ACK = 300;  // Aumentado para dar tempo dos 5 pacotes chegarem

  const int PINO_LED = 2;
  unsigned long tempoUltimoBlink = 0;
  bool estadoLed = LOW;

  uint8_t pacotesEnviados = 0;  // Conta quantos pacotes foram enviados

#else
  // Variáveis exclusivas dos Receptores
  unsigned long tempoDesligarMotor = 0;
  bool motorRodando = false;
  uint8_t contadorPacotes = 0;         // Conta pacotes recebidos
  const uint8_t PACOTES_PARA_ACK = 5; // Envia ACK a cada 5 pacotes
#endif

// Callback de recebimento
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  #ifdef MODO_TRANSMISSOR
    char buffer[len + 1];
    memcpy(buffer, incomingData, len);
    buffer[len] = '\0';
    String msg = String(buffer);

    if (msg == "ACK R") {
      ackRecebidoR = true;
      Serial.println(">>> Confirmação: Rádio DIREITO (R) processou o comando!");
    } 
    else if (msg == "ACK L") {
      ackRecebidoL = true;
      Serial.println(">>> Confirmação: Rádio ESQUERDO (L) processou o comando!");
    }

  #else
    if (len == sizeof(struct_message)) {
      memcpy(&data, incomingData, sizeof(data));
      contadorPacotes++;

      Serial.printf("Pacote %d/5 recebido: Valor %d por %d ms\n",
                    contadorPacotes, data.valor, data.duracao);

      // Aciona o motor normalmente a cada pacote
      analogWrite(pinoPWM, data.valor);
      tempoDesligarMotor = millis() + data.duracao;
      motorRodando = true;

      // Registra o peer na primeira vez
      if (!esp_now_is_peer_exist(mac)) {
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, mac, 6);
        peerInfo.channel = 0;
        peerInfo.encrypt = false;
        esp_now_add_peer(&peerInfo);
      }

      // Só envia ACK a cada 5 pacotes
      if (contadorPacotes >= PACOTES_PARA_ACK) {
        contadorPacotes = 0;  // Reseta o contador

        const char* resposta = "";
        #ifdef MODO_RECEPTOR_DIREITO
          resposta = "ACK R";
        #elif defined(MODO_RECEPTOR_ESQUERDO)
          resposta = "ACK L";
        #endif

        esp_err_t result = esp_now_send(mac, (uint8_t *) resposta, strlen(resposta));

        if (result == ESP_OK) {
          Serial.printf("5 pacotes recebidos — %s ENVIADO.\n", resposta);
        } else {
          Serial.println("Erro interno ao enviar o ACK.");
        }
      }
    }
  #endif
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  #ifdef MODO_TRANSMISSOR
    pinMode(PINO_LED, OUTPUT);
    digitalWrite(PINO_LED, LOW);
  #else
    pinMode(pinoPWM, OUTPUT);
    analogWrite(pinoPWM, 0);
  #endif

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar ESP-NOW");
    return;
  }

  // Registra o endereço de Broadcast
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  esp_now_register_recv_cb(OnDataRecv);

  #ifdef MODO_TRANSMISSOR
    Serial.println("Iniciado como: TRANSMISSOR");
  #elif defined(MODO_RECEPTOR_DIREITO)
    Serial.println("Iniciado como: RECEPTOR DIREITO");
  #elif defined(MODO_RECEPTOR_ESQUERDO)
    Serial.println("Iniciado como: RECEPTOR ESQUERDO");
  #endif
}

void loop() {
  #ifdef MODO_TRANSMISSOR
    if (millis() - tempoUltimoComando > 20 || tempoUltimoComando == 0) {
      data.valor = 200;
      data.duracao = 5000;

      esp_now_send(broadcastAddress, (uint8_t *) &data, sizeof(data));

      pacotesEnviados++;
      tempoUltimoComando = millis();

      Serial.printf("Pacote %d enviado\n", pacotesEnviados);

      // Só verifica ACK a cada 5 pacotes enviados
      if (pacotesEnviados >= 5) {
        pacotesEnviados = 0;
        tempoUltimoEnvio = millis();

        ackRecebidoR = false;
        ackRecebidoL = false;
        erroConexao = false;
      }
    }

    // Verifica timeout só após o 5º pacote
    if (!erroConexao && tempoUltimoEnvio > 0 && (millis() - tempoUltimoEnvio > TIMEOUT_ACK)) {
      if (!ackRecebidoR || !ackRecebidoL) {
        erroConexao = true;
        Serial.println("!!! ALERTA DE DESCONEXÃO !!! Timeout excedido após 5 pacotes.");
        if (!ackRecebidoR) Serial.println(" -> Rádio DIREITO (R) não respondeu.");
        if (!ackRecebidoL) Serial.println(" -> Rádio ESQUERDO (L) não respondeu.");
      }
      tempoUltimoEnvio = 0;  // Reseta para não ficar disparando o alerta
    }

    if (erroConexao) {
      if (millis() - tempoUltimoBlink > 200) {
        tempoUltimoBlink = millis();
        estadoLed = !estadoLed;
        digitalWrite(PINO_LED, estadoLed);
      }
    } else {
      digitalWrite(PINO_LED, LOW);
    }

  #else
    if (motorRodando && millis() > tempoDesligarMotor) {
      analogWrite(pinoPWM, 0);
      motorRodando = false;
      Serial.println("Tempo finalizado: Motor desligado.");
    }
  #endif
}
