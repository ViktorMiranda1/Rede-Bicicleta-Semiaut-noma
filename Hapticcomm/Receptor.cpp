#include "receptor.h"
#if !defined(MODO_TRANSMISSOR) && !defined(MODO_ESP_AUDIO)

#include "config.h"
#include "message.h"
#include "comms.h"
#include <esp_now.h>

// Estado interno do receptor de vibracao
static bool modoPulso = false;             // true quando em estado de STOP
static uint8_t intensidadePulso = 0;       // intensidade usada durante o pulso
static bool pulsoLigado = false;
static unsigned long tempoUltimoPulso = 0;
static uint8_t contadorPacotes = 0;

// Cadencia do pulso no estado de STOP (liga/desliga)
const unsigned long INTERVALO_PULSO = 250; // ms

void receptorSetup() {
  pinMode(pinoPWM, OUTPUT);
  analogWrite(pinoPWM, 0);

  #ifdef MODO_RECEPTOR_DIREITO
    Serial.println("Iniciado como: RECEPTOR DIREITO (V3) - so vibracao");
  #elif defined(MODO_RECEPTOR_ESQUERDO)
    Serial.println("Iniciado como: RECEPTOR ESQUERDO (V3) - so vibracao");
  #endif
}

void receptorProcessarPacote(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len != sizeof(struct_message)) return;

  memcpy(&data, incomingData, sizeof(data));

  // Seleciona a intensidade do lado desta placa (audio e ignorado aqui)
  #ifdef MODO_RECEPTOR_DIREITO
    uint8_t minhaIntensidade = data.intensidadeDireita;
  #elif defined(MODO_RECEPTOR_ESQUERDO)
    uint8_t minhaIntensidade = data.intensidadeEsquerda;
  #else
    uint8_t minhaIntensidade = 0;
  #endif

  switch (data.tipoMensagem) {
    case MSG_START:
      // Sai do modo pulso e desliga o motor: rede ativa
      modoPulso = false;
      analogWrite(pinoPWM, 0);
      Serial.println("START recebido -> rede ativa, saindo do modo STOP.");
      break;

    case MSG_DADOS:
      // Operacao normal: aplica a intensidade do lado (0 = parado)
      modoPulso = false;
      analogWrite(pinoPWM, minhaIntensidade);
      Serial.printf("DADOS -> Vibracao: %d/255\n", minhaIntensidade);
      break;

    case MSG_STOP:
      // Perda de conexao: entra em vibracao por pulso (feita no loop)
      modoPulso = true;
      intensidadePulso = (minhaIntensidade > 0) ? minhaIntensidade : 200;
      Serial.println("STOP -> vibracao em pulso.");
      break;

    default:
      return; // tipo desconhecido: ignora
  }

  // Monitoramento de conexao: responde ACK periodicamente
  contadorPacotes++;
  commsRegistrarPeer(mac);

  if (contadorPacotes >= PACOTES_PARA_ACK) {
    contadorPacotes = 0;

    const char* resposta = "";
    #ifdef MODO_RECEPTOR_DIREITO
      resposta = "ACK R";
    #elif defined(MODO_RECEPTOR_ESQUERDO)
      resposta = "ACK L";
    #endif

    esp_err_t result = esp_now_send(mac, (uint8_t *) resposta, strlen(resposta));

    if (result == ESP_OK) {
      Serial.printf("%d pacotes recebidos - %s ENVIADO.\n", PACOTES_PARA_ACK, resposta);
    } else {
      Serial.println("Erro interno ao enviar o ACK.");
    }
  }
}

void receptorLoop() {
  // Gera o pulso enquanto estiver em estado de STOP
  if (modoPulso) {
    if (millis() - tempoUltimoPulso > INTERVALO_PULSO) {
      tempoUltimoPulso = millis();
      pulsoLigado = !pulsoLigado;
      analogWrite(pinoPWM, pulsoLigado ? intensidadePulso : 0);
    }
  }
}

#endif // !MODO_TRANSMISSOR && !MODO_ESP_AUDIO