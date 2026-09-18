#include "espaudio.h"
#ifdef MODO_ESP_AUDIO

#include "config.h"
#include "message.h"
#include "comms.h"
#include <esp_now.h>

// Biblioteca do modulo de audio.
// Instale "DFRobotDFPlayerMini" pelo Gerenciador de Bibliotecas do Arduino.
#include <DFRobotDFPlayerMini.h>

static HardwareSerial dfSerial(2);         // UART2 da ESP32
static DFRobotDFPlayerMini dfPlayer;
static bool dfOk = false;

// Evita reiniciar o mesmo audio a cada pacote (o transmissor envia em rajada).
// O mesmo audio so e re-disparado depois de INTERVALO_MIN_AUDIO.
static uint8_t ultimaPasta = 0;
static uint8_t ultimoArquivo = 0;
static unsigned long tempoUltimoAudio = 0;
const unsigned long INTERVALO_MIN_AUDIO = 800; // ms

void espAudioSetup() {
  dfSerial.begin(9600, SERIAL_8N1, PINO_DFPLAYER_RX, PINO_DFPLAYER_TX);
  Serial.println("Iniciado como: ESP AUDIOS - so audio");

  if (dfPlayer.begin(dfSerial)) {
    dfOk = true;
    dfPlayer.volume(VOLUME_AUDIO);   // 0 a 30
    Serial.println("DFPlayer Mini pronto.");
  } else {
    Serial.println("Falha ao iniciar o DFPlayer Mini (verifique fiacao e cartao SD).");
  }
}

// Toca um audio (pasta/arquivo) evitando repeticao pela rajada de pacotes.
static void tocar(uint8_t pasta, uint8_t arquivo) {
  if (pasta == 0 || arquivo == 0) return;  // 0 = sem audio

  bool mesmoAudio = (pasta == ultimaPasta && arquivo == ultimoArquivo);
  if (mesmoAudio && (millis() - tempoUltimoAudio < INTERVALO_MIN_AUDIO)) return;

  ultimaPasta = pasta;
  ultimoArquivo = arquivo;
  tempoUltimoAudio = millis();

  Serial.printf("Tocando audio -> Pasta %d, Arquivo %d\n", pasta, arquivo);
  if (dfOk) dfPlayer.playFolder(pasta, arquivo);
}

void espAudioProcessarPacote(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len != sizeof(struct_message)) return;

  memcpy(&data, incomingData, sizeof(data));

  // A ESP de audio usa APENAS os campos de audio (ignora as intensidades).
  switch (data.tipoMensagem) {
    case MSG_START:
      // Rede iniciada: libera para tocar novos audios
      ultimaPasta = 0;
      ultimoArquivo = 0;
      Serial.println("START recebido -> ESP Audios pronta.");
      break;

    case MSG_DADOS:
    case MSG_STOP:
      // Toca o audio indicado (se houver). No STOP e o alerta de perda de conexao.
      tocar(data.pastaAudio, data.arquivoAudio);
      break;

    default:
      break;
  }
}

void espAudioLoop() {
  // A reproducao e disparada na recepcao; nada continuo por aqui.
}

#endif // MODO_ESP_AUDIO