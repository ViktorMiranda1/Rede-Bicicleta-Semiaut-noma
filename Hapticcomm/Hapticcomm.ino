#include "config.h"
#include "comms.h"

#ifdef MODO_TRANSMISSOR
  #include "transmissor.h"
#else
  #include "receptor.h"
#endif

void setup() {
  Serial.begin(115200);
  delay(1000); // Aguarda estabilização da serial

  if (commsInit()) {
    Serial.println("Comunicação iniciada!");
  } else {
    Serial.println("Falha ao iniciar ESP-NOW");
  }

#ifdef MODO_TRANSMISSOR
  transmissorSetup();
#else
  receptorSetup();
#endif
}

void loop() {
#ifdef MODO_TRANSMISSOR
  transmissorLoop();
#else
  receptorLoop();
#endif
}