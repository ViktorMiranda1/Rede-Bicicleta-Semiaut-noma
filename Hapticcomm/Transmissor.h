#ifndef TRANSMISSOR_H
#define TRANSMISSOR_H

#include "config.h"
#ifdef MODO_TRANSMISSOR

#include <Arduino.h>

void transmissorSetup();
void transmissorLoop();
void transmissorProcessarACK(const uint8_t *mac, const uint8_t *incomingData, int len);

#endif // MODO_TRANSMISSOR
#endif // TRANSMISSOR_H