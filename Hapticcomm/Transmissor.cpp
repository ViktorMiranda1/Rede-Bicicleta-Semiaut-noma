#include "transmissor.h"
#ifdef MODO_TRANSMISSOR

#include "config.h"
#include "message.h"
#include <esp_now.h>

// Estado interno do transmissor
static bool ackRecebidoR = false;
static bool ackRecebidoL = false;
static bool erroConexao  = false;

static unsigned long tempoUltimoComando = 0;
static unsigned long tempoUltimoEnvio   = 0;

static unsigned long tempoUltimoBlink = 0;
static bool estadoLed = LOW;

static uint8_t pacotesEnviados = 0;

void transmissorSetup() {
  pinMode(PINO_LED, OUTPUT);
  digitalWrite(PINO_LED, LOW);
  Serial.println("Iniciado como: TRANSMISSOR (V2)");
}

void transmissorProcessarACK(const uint8_t *mac, const uint8_t *incomingData, int len) {
  char buffer[len + 1];
  memcpy(buffer, incomingData, len);
  buffer[len] = '\0';
  String msg = String(buffer);

  if (msg == "ACK R") {
    ackRecebidoR = true;
    Serial.println(">>> Confirmacao: Radio DIREITO (R) processou o comando!");
  } else if (msg == "ACK L") {
    ackRecebidoL = true;
    Serial.println(">>> Confirmacao: Radio ESQUERDO (L) processou o comando!");
  }
}

void transmissorLoop() {
  if (millis() - tempoUltimoComando > INTERVALO_ENVIO || tempoUltimoComando == 0) {
    // Configura os dados do comando V2
    data.audioAtivo          = 0;     // 0 = Desligado (ou 1 se ainda quiser o áudio)
    data.intensidadeVibracao = 128;   // Intensidade média (~50% do PWM de 0 a 255)
    data.duracao             = 5;     // Duração: 5 ms
    data.ladoDireito         = true;  // Envia para o receptor direito
    data.ladoEsquerdo        = true;  // Envia para o receptor esquerdo
    data.pastaAudio          = 0x01;
    data.arquivoAudio        = 0x012C;

    esp_now_send(broadcastAddress, (uint8_t *) &data, sizeof(data));

    pacotesEnviados++;
    tempoUltimoComando = millis();

    Serial.printf("Pacote %d enviado (Vib: 0x%02X, Dur: %d ms, Aud: %d [P:0x%02X, A:0x%04X (%d)], Lados: R=%d L=%d)\n",
                  pacotesEnviados, data.intensidadeVibracao, data.duracao,
                  data.audioAtivo, data.pastaAudio, data.arquivoAudio, data.arquivoAudio,
                  data.ladoDireito, data.ladoEsquerdo);

    // Só verifica ACK a cada ciclo de pacotes enviados
    if (pacotesEnviados >= PACOTES_POR_CICLO) {
      pacotesEnviados = 0;
      tempoUltimoEnvio = millis();

      ackRecebidoR = false;
      ackRecebidoL = false;
      erroConexao  = false;
    }
  }

  // Verifica timeout só após o ciclo (apenas para os lados requisitados)
  if (!erroConexao && tempoUltimoEnvio > 0 && (millis() - tempoUltimoEnvio > TIMEOUT_ACK)) {
    bool falhaR = data.ladoDireito  && !ackRecebidoR;
    bool falhaL = data.ladoEsquerdo && !ackRecebidoL;

    if (falhaR || falhaL) {
      erroConexao = true;
      Serial.println("!!! ALERTA DE DESCONEXAO !!! Timeout excedido apos o ciclo de pacotes.");
      if (falhaR) Serial.println(" -> Radio DIREITO (R) nao respondeu.");
      if (falhaL) Serial.println(" -> Radio ESQUERDO (L) nao respondeu.");
    }
    tempoUltimoEnvio = 0; // Reseta para não ficar disparando o alerta
  }

  // LED de status
  if (erroConexao) {
    if (millis() - tempoUltimoBlink > 200) {
      tempoUltimoBlink = millis();
      estadoLed = !estadoLed;
      digitalWrite(PINO_LED, estadoLed);
    }
  } else {
    digitalWrite(PINO_LED, LOW);
  }
}

#endif // MODO_TRANSMISSOR