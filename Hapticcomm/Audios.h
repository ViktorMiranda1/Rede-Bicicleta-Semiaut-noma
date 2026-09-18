#ifndef AUDIOS_H
#define AUDIOS_H

#include <Arduino.h>

// ============================================================
//  TABELA DE AUDIOS
//  Cada audio e um par (pasta, arquivo) no cartao do DFPlayer.
//
//  ATENCAO: os numeros de pasta/arquivo abaixo sao PLACEHOLDERS
//  inventados. Quando a equipe gravar os audios de verdade, e so
//  trocar os pares { pasta, arquivo } de cada linha.
//
//  PARA ADICIONAR UM AUDIO NOVO:
//    1) crie um nome na enum AudioID (antes de AUDIO_TOTAL)
//    2) adicione a linha correspondente em TABELA_AUDIOS
//  A enum e a tabela precisam ficar na MESMA ORDEM.
//
//  Para usar: data.pastaAudio   = AUDIO_PASTA(AUDIO_VIRAR_DIREITA);
//             data.arquivoAudio = AUDIO_ARQUIVO(AUDIO_VIRAR_DIREITA);
// ============================================================

// Referencia de um audio: pasta e arquivo no DFPlayer
typedef struct {
  uint8_t pasta;
  uint8_t arquivo;
} AudioRef;

// Identificadores dos audios (funcionam como indice da tabela)
enum AudioID {
  // ---- Pasta 1: Sistema / Conexao ----
  AUDIO_PERDA_DIREITA = 0,   // "Perda de conexao a direita"
  AUDIO_PERDA_ESQUERDA,      // "Perda de conexao a esquerda"
  AUDIO_PERDA_AMBOS,         // "Perda de conexao (ambos os lados)"
  AUDIO_REDE_INICIADA,       // "Sistema conectado"
  AUDIO_SISTEMA_PRONTO,      // "Sistema pronto para uso"
  AUDIO_BATERIA_BAIXA,       // "Bateria baixa"

  // ---- Pasta 2: Navegacao ----
  AUDIO_VIRAR_DIREITA,       // "Vire a direita"
  AUDIO_VIRAR_ESQUERDA,      // "Vire a esquerda"
  AUDIO_SEGUIR_FRENTE,       // "Siga em frente"
  AUDIO_REDUZIR_VELOCIDADE,  // "Reduza a velocidade"
  AUDIO_CHEGOU_DESTINO,      // "Voce chegou ao destino"

  // ---- Pasta 3: Obstaculos ----
  AUDIO_OBSTACULO_FRENTE,    // "Obstaculo a frente"
  AUDIO_OBSTACULO_DIREITA,   // "Obstaculo a direita"
  AUDIO_OBSTACULO_ESQUERDA,  // "Obstaculo a esquerda"

  // >>> adicione novos audios aqui <<<

  AUDIO_TOTAL                // Mantenha SEMPRE por ultimo (conta os itens)
};

// Tabela: para cada AudioID, a pasta e o arquivo correspondentes.
// Mantenha as linhas na MESMA ORDEM da enum acima.
const AudioRef TABELA_AUDIOS[AUDIO_TOTAL] = {
  // ---- Pasta 1: Sistema / Conexao ----
  /* AUDIO_PERDA_DIREITA      */ { 1, 1 },
  /* AUDIO_PERDA_ESQUERDA     */ { 1, 2 },
  /* AUDIO_PERDA_AMBOS        */ { 1, 3 },
  /* AUDIO_REDE_INICIADA      */ { 1, 4 },
  /* AUDIO_SISTEMA_PRONTO     */ { 1, 5 },
  /* AUDIO_BATERIA_BAIXA      */ { 1, 6 },

  // ---- Pasta 2: Navegacao ----
  /* AUDIO_VIRAR_DIREITA      */ { 2, 1 },
  /* AUDIO_VIRAR_ESQUERDA     */ { 2, 2 },
  /* AUDIO_SEGUIR_FRENTE      */ { 2, 3 },
  /* AUDIO_REDUZIR_VELOCIDADE */ { 2, 4 },
  /* AUDIO_CHEGOU_DESTINO     */ { 2, 5 },

  // ---- Pasta 3: Obstaculos ----
  /* AUDIO_OBSTACULO_FRENTE   */ { 3, 1 },
  /* AUDIO_OBSTACULO_DIREITA  */ { 3, 2 },
  /* AUDIO_OBSTACULO_ESQUERDA */ { 3, 3 },

  // >>> adicione novas linhas aqui, na mesma ordem da enum <<<
};

// Atalhos para preencher os campos da mensagem a partir de um AudioID
#define AUDIO_PASTA(id)   (TABELA_AUDIOS[(id)].pasta)
#define AUDIO_ARQUIVO(id) (TABELA_AUDIOS[(id)].arquivo)

#endif // AUDIOS_H