#ifndef MESSAGE_H
#define MESSAGE_H

#include <Arduino.h>
#include "audios.h"   // Tabela de audios (enum AudioID + TABELA_AUDIOS)

// ============================================================
//  TIPOS DE MENSAGEM (campo tipoMensagem)
// ============================================================
#define MSG_START  0x00  // Inicia a rede / sai do estado de Stop
#define MSG_DADOS  0x01  // Mensagem normal (vibracao + audio) [o que ja existia]
#define MSG_STOP   0x02  // Perda de conexao -> vibracao em pulso + audio de alerta

// ============================================================
//  ESTRUTURA DE DADOS V3 (Tamanho total: 5 bytes)
// ------------------------------------------------------------
//  Como cada tipo interpreta os campos:
//
//  MSG_START (0x00): pacote de controle. Tira os receptores de um
//    eventual estado de Stop e ativa a rede. Intensidades = 0 e
//    pasta/arquivo = 0 (sem audio).
//
//  MSG_DADOS (0x01): operacao normal. Cada receptor usa a
//    intensidade do seu lado (direita/esquerda); 0 = lado parado.
//    Se pastaAudio e arquivoAudio forem > 0, toca o audio indicado.
//
//  MSG_STOP  (0x02): perda de conexao. Os receptores vibram em
//    pulso (a cadencia e gerada no proprio receptor) e tocam o
//    audio de alerta apontado por pastaAudio/arquivoAudio, que
//    identifica se a queda foi na esquerda, na direita ou em ambos.
//
//  Para montar os campos de audio, use os atalhos da tabela:
//    data.pastaAudio   = AUDIO_PASTA(AUDIO_VIRAR_DIREITA);
//    data.arquivoAudio = AUDIO_ARQUIVO(AUDIO_VIRAR_DIREITA);
// ============================================================
typedef struct __attribute__((packed)) {
    uint8_t tipoMensagem;         // MSG_START | MSG_DADOS | MSG_STOP (1 byte)
    uint8_t intensidadeDireita;   // PWM lado direito  (0x00 a 0xFF) (1 byte)
    uint8_t intensidadeEsquerda;  // PWM lado esquerdo (0x00 a 0xFF) (1 byte)
    uint8_t pastaAudio;           // Numero da pasta de audio (1 byte)
    uint8_t arquivoAudio;         // Numero do arquivo de audio (1 byte)
} struct_message;

extern struct_message data;

#endif // MESSAGE_H