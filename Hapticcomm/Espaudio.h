#ifndef ESPAUDIO_H
#define ESPAUDIO_H

#include "config.h"
#ifdef MODO_ESP_AUDIO

#include <Arduino.h>

void espAudioSetup();
void espAudioLoop();
void espAudioProcessarPacote(const uint8_t *mac, const uint8_t *incomingData, int len);

#endif // MODO_ESP_AUDIO
#endif // ESPAUDIO_H