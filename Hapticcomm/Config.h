#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================
//  SELECAO DE MODO
//  Descomente APENAS UMA das linhas abaixo para escolher a placa
// ============================================================
#define MODO_TRANSMISSOR
//#define MODO_RECEPTOR_DIREITO   // ESP do lado direito  -> so vibracao (intensidadeDireita)
//#define MODO_RECEPTOR_ESQUERDO  // ESP do lado esquerdo -> so vibracao (intensidadeEsquerda)
//#define MODO_ESP_AUDIO          // ESP de audio -> so audio (pastaAudio / arquivoAudio)

// ============================================================
//  HARDWARE COMUM
// ============================================================
const int pinoPWM = 18;   // Usado apenas pelos receptores de vibracao
extern uint8_t broadcastAddress[6];

// ============================================================
//  CONSTANTES DO TRANSMISSOR
// ============================================================
#ifdef MODO_TRANSMISSOR
  const int PINO_LED = 2;
  const unsigned long TIMEOUT_ACK = 300;   // Tempo p/ chegada das confirmacoes (ms)
  const uint8_t PACOTES_POR_CICLO = 5;     // Verifica ACK a cada N pacotes
  const unsigned long INTERVALO_ENVIO = 20; // Intervalo entre envios (ms)

// ============================================================
//  CONSTANTES DA ESP DE AUDIO
// ============================================================
#elif defined(MODO_ESP_AUDIO)
  const int PINO_DFPLAYER_RX = 16;   // RX da ESP  <- TX do DFPlayer
  const int PINO_DFPLAYER_TX = 17;   // TX da ESP  -> RX do DFPlayer
  const uint8_t VOLUME_AUDIO = 25;   // Volume do DFPlayer (0 a 30)

// ============================================================
//  CONSTANTES DOS RECEPTORES DE VIBRACAO
// ============================================================
#else
  const uint8_t PACOTES_PARA_ACK = 5;      // Envia ACK a cada N pacotes recebidos
#endif

#endif // CONFIG_H