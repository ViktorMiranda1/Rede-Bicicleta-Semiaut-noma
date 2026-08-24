#include <esp_now.h>
#include <WiFi.h>

// MODOS: Descomente APENAS UMA das linhas abaixo para escolher a placa
#define MODO_TRANSMISSOR 
//#define MODO_RECEPTOR_DIREITO
//#define MODO_RECEPTOR_ESQUERDO

// Configurações de hardware comuns
const int pinoPWM = 18;
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Estrutura de dados comum (Tamanho total: 8 bytes)
typedef struct __attribute__((packed)) {
    uint8_t audioAtivo;            // 0 = Desligado | 1 = Tocar Áudio
    uint8_t intensidadeVibracao;   // 0x00 a 0xFF (0 a 255) - Potência do PWM de vibração
    uint16_t duracao;              // Duração da vibração em milissegundos (0 a 65.535 ms)
    bool ladoDireito;              // true (1) = Aciona lado Direito | false (0)
    bool ladoEsquerdo;             // true (1) = Aciona lado Esquerdo | false (0)
    uint8_t pastaAudio;            // Número da pasta (ex: 0x01 a 0x63 / 1 a 99)
    uint8_t arquivoAudio;          // Identificador do arquivo/faixa (0x00 a 0xFF / 0 a 255)
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

      // Filtra se a mensagem é destinada a esta placa
      #ifdef MODO_RECEPTOR_DIREITO
        if (!data.ladoDireito) return; // Ignora se o comando não for para o lado direito
      #elif defined(MODO_RECEPTOR_ESQUERDO)
        if (!data.ladoEsquerdo) return; // Ignora se o comando não for para o lado esquerdo
      #endif

      contadorPacotes++;

      Serial.printf("Pacote %d/5 recebido -> Vibracao: 0x%02X (%d/255), Duracao: %d ms, Audio: %d (Pasta: 0x%02X, Faixa: 0x%02X)\n",
                    contadorPacotes, data.intensidadeVibracao, data.intensidadeVibracao, data.duracao,
                    data.audioAtivo, data.pastaAudio, data.arquivoAudio);

      // Aciona o motor de vibração normalmente a cada pacote
      analogWrite(pinoPWM, data.intensidadeVibracao);
      tempoDesligarMotor = millis() + data.duracao;
      motorRodando = true;

      // Execução de áudio (se ativado)
      if (data.audioAtivo == 1) {
        // Espaço reservado para acionamento do módulo de áudio (ex: DFPlayer Mini)
        // myDFPlayer.playFolder(data.pastaAudio, data.arquivoAudio);
      }

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
      // Configura os dados do comando
      data.audioAtivo = 1;
      data.intensidadeVibracao = 0xC8; // Ex: 200 (0xC8 em hex)
      data.duracao = 5000;             // 5000 ms (5 segundos)
      data.ladoDireito = true;         // Habilita lado direito
      data.ladoEsquerdo = true;        // Habilita lado esquerdo
      data.pastaAudio = 0x01;          // Pasta 0x01
      data.arquivoAudio = 0x01;        // Arquivo 0x01

      esp_now_send(broadcastAddress, (uint8_t *) &data, sizeof(data));

      pacotesEnviados++;
      tempoUltimoComando = millis();

      Serial.printf("Pacote %d enviado (Vib: 0x%02X, Dur: %d ms, Aud: %d [P:0x%02X, A:0x%02X], Lados: R=%d L=%d)\n",
                    pacotesEnviados, data.intensidadeVibracao, data.duracao,
                    data.audioAtivo, data.pastaAudio, data.arquivoAudio,
                    data.ladoDireito, data.ladoEsquerdo);

      // Só verifica ACK a cada 5 pacotes enviados
      if (pacotesEnviados >= 5) {
        pacotesEnviados = 0;
        tempoUltimoEnvio = millis();

        ackRecebidoR = false;
        ackRecebidoL = false;
        erroConexao = false;
      }
    }

    // Verifica timeout só após o 5º pacote (apenas para os lados requisitados)
    if (!erroConexao && tempoUltimoEnvio > 0 && (millis() - tempoUltimoEnvio > TIMEOUT_ACK)) {
      bool falhaR = data.ladoDireito && !ackRecebidoR;
      bool falhaL = data.ladoEsquerdo && !ackRecebidoL;

      if (falhaR || falhaL) {
        erroConexao = true;
        Serial.println("!!! ALERTA DE DESCONEXÃO !!! Timeout excedido após 5 pacotes.");
        if (falhaR) Serial.println(" -> Rádio DIREITO (R) não respondeu.");
        if (falhaL) Serial.println(" -> Rádio ESQUERDO (L) não respondeu.");
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
      Serial.println("Tempo finalizado: Motor de vibração desligado.");
    }
  #endif
}
