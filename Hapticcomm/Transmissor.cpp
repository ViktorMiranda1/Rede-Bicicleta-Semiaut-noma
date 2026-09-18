#include "transmissor.h"
#ifdef MODO_TRANSMISSOR

#include "config.h"
#include "message.h"
#include <esp_now.h>

// Estado interno do transmissor
static bool ackRecebidoR = false;
static bool ackRecebidoL = false;
static bool erroConexao  = false;

// Guarda qual lado falhou para escolher o audio de alerta no STOP
static bool falhaR = false;
static bool falhaL = false;

static unsigned long tempoUltimoComando = 0;
static unsigned long tempoUltimoEnvio   = 0;

static unsigned long tempoUltimoBlink = 0;
static bool estadoLed = LOW;

static uint8_t pacotesEnviados = 0;

// Envia um pacote de controle START (inicia a rede / sai do STOP)
static void enviarStart() {
  data.tipoMensagem        = MSG_START;
  data.intensidadeDireita  = 0;
  data.intensidadeEsquerda = 0;
  data.pastaAudio          = 0;
  data.arquivoAudio        = 0;
  esp_now_send(broadcastAddress, (uint8_t *) &data, sizeof(data));
}

void transmissorSetup() {
  pinMode(PINO_LED, OUTPUT);
  digitalWrite(PINO_LED, LOW);
  Serial.println("Iniciado como: TRANSMISSOR (V3)");

  // Anuncia o inicio da rede
  enviarStart();
  Serial.println("Comando START enviado (rede iniciada).");
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

    if (erroConexao) {
      // Estado de STOP: pulso em ambos os lados + audio de perda de conexao.
      // O audio e escolhido pela TABELA_AUDIOS conforme o lado que caiu.
      AudioID alerta;
      if (falhaR && falhaL)      alerta = AUDIO_PERDA_AMBOS;
      else if (falhaR)           alerta = AUDIO_PERDA_DIREITA;
      else                       alerta = AUDIO_PERDA_ESQUERDA;

      data.tipoMensagem        = MSG_STOP;
      data.intensidadeDireita  = 200;   // nivel usado no pulso
      data.intensidadeEsquerda = 200;
      data.pastaAudio          = AUDIO_PASTA(alerta);
      data.arquivoAudio        = AUDIO_ARQUIVO(alerta);
    } else {
      // Operacao normal: mensagem de dados
      data.tipoMensagem        = MSG_DADOS;
      data.intensidadeDireita  = 128;   // ~50% do PWM no lado direito
      data.intensidadeEsquerda = 128;   // ~50% do PWM no lado esquerdo
      data.pastaAudio          = 0;     // 0 = sem audio
      data.arquivoAudio        = 0;
    }

    esp_now_send(broadcastAddress, (uint8_t *) &data, sizeof(data));

    pacotesEnviados++;
    tempoUltimoComando = millis();

    Serial.printf("Pacote %d enviado (Tipo: 0x%02X | Dir: %d | Esq: %d | Audio P:%d A:%d)\n",
                  pacotesEnviados, data.tipoMensagem,
                  data.intensidadeDireita, data.intensidadeEsquerda,
                  data.pastaAudio, data.arquivoAudio);

    // So verifica ACK a cada ciclo de pacotes enviados
    if (pacotesEnviados >= PACOTES_POR_CICLO) {
      pacotesEnviados = 0;
      tempoUltimoEnvio = millis();

      ackRecebidoR = false;
      ackRecebidoL = false;
    }
  }

  // Avalia os ACKs apos o ciclo
  if (tempoUltimoEnvio > 0 && (millis() - tempoUltimoEnvio > TIMEOUT_ACK)) {
    falhaR = !ackRecebidoR;
    falhaL = !ackRecebidoL;
    bool houveFalha = falhaR || falhaL;

    if (houveFalha && !erroConexao) {
      // Entra em modo STOP
      erroConexao = true;
      Serial.println("!!! ALERTA DE DESCONEXAO !!! -> entrando em modo STOP.");
      if (falhaR) Serial.println(" -> Radio DIREITO (R) nao respondeu.");
      if (falhaL) Serial.println(" -> Radio ESQUERDO (L) nao respondeu.");
    } else if (!houveFalha && erroConexao) {
      // Conexao restabelecida -> sai do STOP e reinicia a rede
      erroConexao = false;
      falhaR = false;
      falhaL = false;
      enviarStart();
      Serial.println(">>> Conexao restabelecida: comando START reenviado.");
    }

    tempoUltimoEnvio = 0; // Reseta para nao ficar reavaliando o mesmo ciclo
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