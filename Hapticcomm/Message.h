#ifndef MESSAGE_H
#define MESSAGE_H

#include <Arduino.h>

// ============================================================
//  ESTRUTURA DE DADOS V2 (Tamanho total: 9 bytes)
// ============================================================
typedef struct __attribute__((packed)) {
    uint8_t  audioAtivo;          // 0 = Desligado | 1 = Tocar Áudio (1 byte)
    uint8_t  intensidadeVibracao; // 0x00 a 0xFF (0 a 255) - Potência do PWM (1 byte)
    uint16_t duracao;             // Duração da vibração em ms (0 a 65.535) (2 bytes)
    bool     ladoDireito;         // true = Aciona lado Direito (1 byte)
    bool     ladoEsquerdo;        // true = Aciona lado Esquerdo (1 byte)
    uint8_t  pastaAudio;          // Número da pasta (1 a 99) (1 byte)
    uint16_t arquivoAudio;        // Identificador do arquivo/faixa (0 a 65.535) (2 bytes)
} struct_message;

extern struct_message data;

#endif // MESSAGE_H