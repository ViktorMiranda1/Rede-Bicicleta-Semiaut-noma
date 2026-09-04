#include "receptor.h"
#if !defined(MODO_TRANSMISSOR)

#include "config.h"
#include "message.h"
#include "comms.h"
#include <esp_now.h>

// Estado interno do receptor
static unsigned long tempoDesligarMotor = 0;
static bool motorRodando = false;
static uint8_t contadorPacotes = 0;

void receptorSetup() {
  pinMode(pinoPWM, OUTPUT);
  analogWrite(pinoPWM, 0);

  #ifdef MODO_RECEPTOR_DIREITO
    Serial.println("Iniciado como: RECEPTOR DIREITO (V2)");
  #elif defined(MODO_RECEPTOR_ESQUERDO)
    Serial.println("Iniciado como: RECEPTOR ESQUERDO (V2)");
  #endif
}

void receptorProcessarPacote(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len != sizeof(struct_message)) return;

  memcpy(&data, incomingData, sizeof(data));

  // Filtra se a mensagem é destinada a esta placa
  #ifdef MODO_RECEPTOR_DIREITO
    if (!data.ladoDireito) return;
  #elif defined(MODO_RECEPTOR_ESQUERDO)
    if (!data.ladoEsquerdo) return;
  #endif

  contadorPacotes++;

  Serial.printf("Pacote %d/%d recebido -> Vibracao: 0x%02X (%d/255), Duracao: %d ms, Audio: %d (Pasta: 0x%02X, Faixa: 0x%04X [%d])\n",
                contadorPacotes, PACOTES_PARA_ACK, data.intensidadeVibracao, data.intensidadeVibracao, data.duracao,
                data.audioAtivo, data.pastaAudio, data.arquivoAudio, data.arquivoAudio);

  // Aciona o motor de vibração a cada pacote
  analogWrite(pinoPWM, data.intensidadeVibracao);
  tempoDesligarMotor = millis() + data.duracao;
  motorRodando = true;

  // Execução de áudio (se ativado)
  if (data.audioAtivo == 1) {
    // Espaço reservado para acionamento do módulo de áudio (ex: DFPlayer Mini)
    // myDFPlayer.playFolder(data.pastaAudio, data.arquivoAudio);
  }

  // Registra o peer na primeira vez
  commsRegistrarPeer(mac);

  // Só envia ACK a cada N pacotes
  if (contadorPacotes >= PACOTES_PARA_ACK) {
    contadorPacotes = 0;

    const char* resposta = "";
    #ifdef MODO_RECEPTOR_DIREITO
      resposta = "ACK R";
    #elif defined(MODO_RECEPTOR_ESQUERDO)
      resposta = "ACK L";
    #endif

    esp_err_t result = esp_now_send(mac, (uint8_t *) resposta, strlen(resposta));

    if (result == ESP_OK) {
      Serial.printf("%d pacotes recebidos - %s ENVIADO.\n", PACOTES_PARA_ACK, resposta);
    } else {
      Serial.println("Erro interno ao enviar o ACK.");
    }
  }
}

void receptorLoop() {
  if (motorRodando && millis() > tempoDesligarMotor) {
    analogWrite(pinoPWM, 0);
    motorRodando = false;
    Serial.println("Tempo finalizado: Motor de vibracao desligado.");
  }
}

#endif // !MODO_TRANSMISSOR