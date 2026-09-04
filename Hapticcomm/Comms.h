#ifndef COMMS_H
#define COMMS_H

#include <Arduino.h>
#include <esp_now.h>

// Inicializa WiFi + ESP-NOW, registra peer de broadcast e callback de RX.
// Retorna true em caso de sucesso.
bool commsInit();

// Registra um peer (unicast) caso ainda não exista.
void commsRegistrarPeer(const uint8_t *mac);

// Callback de recebimento do ESP-NOW (assinatura do core ESP32 3.x).
void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len);

#endif // COMMS_H