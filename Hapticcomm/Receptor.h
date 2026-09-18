#ifndef RECEPTOR_H
#define RECEPTOR_H

#include "config.h"
#if !defined(MODO_TRANSMISSOR) && !defined(MODO_ESP_AUDIO)

#include <Arduino.h>

void receptorSetup();
void receptorLoop();
void receptorProcessarPacote(const uint8_t *mac, const uint8_t *incomingData, int len);

#endif // !MODO_TRANSMISSOR && !MODO_ESP_AUDIO
#endif // RECEPTOR_H