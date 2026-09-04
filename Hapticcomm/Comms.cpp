#include "comms.h"
#include "config.h"
#include "message.h"
#include <esp_now.h>
#include <WiFi.h>

#ifdef MODO_TRANSMISSOR
  #include "transmissor.h"
#else
  #include "receptor.h"
#endif

// Definições das variáveis globais declaradas como extern
uint8_t broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
struct_message data;

void commsRegistrarPeer(const uint8_t *mac) {
  if (!esp_now_is_peer_exist(mac)) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, mac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
  }
}

void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  const uint8_t *mac = recv_info->src_addr;
  #ifdef MODO_TRANSMISSOR
    transmissorProcessarACK(mac, incomingData, len);
  #else
    receptorProcessarPacote(mac, incomingData, len);
  #endif
}

bool commsInit() {
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar ESP-NOW");
    return false;
  }

  // Registra o endereço de Broadcast
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  esp_now_register_recv_cb(OnDataRecv);
  return true;
}