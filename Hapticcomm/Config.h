#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================
//  SELEÇÃO DE MODO
//  Descomente APENAS UMA das linhas abaixo para escolher a placa
// ============================================================
//#define MODO_TRANSMISSOR
#define MODO_RECEPTOR_DIREITO
//#define MODO_RECEPTOR_ESQUERDO

// ============================================================
//  HARDWARE COMUM
// ============================================================
const int pinoPWM = 18;
extern uint8_t broadcastAddress[6];

// ============================================================
//  CONSTANTES DO TRANSMISSOR
// ============================================================
#ifdef MODO_TRANSMISSOR
  const int PINO_LED = 2;
  const unsigned long TIMEOUT_ACK = 300;   // Tempo p/ chegada das confirmações (ms)
  const uint8_t PACOTES_POR_CICLO = 5;     // Verifica ACK a cada N pacotes
  const unsigned long INTERVALO_ENVIO = 20; // Intervalo entre envios (ms)
#else
// ============================================================
//  CONSTANTES DOS RECEPTORES
// ============================================================
  const uint8_t PACOTES_PARA_ACK = 5;      // Envia ACK a cada N pacotes recebidos
#endif

#endif // CONFIG_H