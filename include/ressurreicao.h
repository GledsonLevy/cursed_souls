#ifndef RESSURREICAO_H
#define RESSURREICAO_H

#include "regras.h"

#define BONUS_CORRENTES 20

typedef enum {
    RESS_FALHA,
    RESS_SUCESSO_EQUIPE,
    RESS_SUCESSO_ARMAZENAMENTO,
    RESS_ERRO
} ResultadoRess;

int ressurreicaoChance(Raridade r, int usarCorrentes);   /* em % (1..95) */
int ressurreicaoSortear(void);                           /* 0..99 */
/* sorteio < chance => sucesso. Consome 1 carga de Correntes se usarCorrentes. */
ResultadoRess ressurreicaoTentar(EstadoJogo *g, const Criatura *derrotada,
                                 int usarCorrentes, int sorteio);

#endif
