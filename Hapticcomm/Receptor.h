#ifndef RECEPTOR_H
#define RECEPTOR_H

#include "config.h"
#if !defined(MODO_TRANSMISSOR)

#include <Arduino.h>

void receptorSetup();
void receptorLoop();
void receptorProcessarPacote(const uint8_t *mac, const uint8_t *incomingData, int len);

#endif // !MODO_TRANSMISSOR
#endif // RECEPTOR_H