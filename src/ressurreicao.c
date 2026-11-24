#include "ressurreicao.h"
#include <stdlib.h>

static const int CHANCE_BASE[RAR_TOTAL] = {75, 55, 35, 20, 10};

int ressurreicaoChance(Raridade r, int usarCorrentes)
{
    if (r < 0 || r >= RAR_TOTAL) return 1;
    int chance = CHANCE_BASE[r] + (usarCorrentes ? BONUS_CORRENTES : 0);
    return limitar(chance, 1, 95);
}

int ressurreicaoSortear(void)
{
    return rand() % 100;
}

ResultadoRess ressurreicaoTentar(EstadoJogo *g, const Criatura *derrotada,
                                 int usarCorrentes, int sorteio)
{
    if (g == NULL || derrotada == NULL || sorteio < 0 || sorteio > 99) return RESS_ERRO;
    if (usarCorrentes && g->jogador.correntes <= 0) return RESS_ERRO;

    int chance = ressurreicaoChance(derrotada->raridade, usarCorrentes);
    if (usarCorrentes) g->jogador.correntes--;

    if (sorteio >= chance) return RESS_FALHA;

    Criatura nova = *derrotada;
    nova.hp = nova.hpMax / 2;
    if (nova.hp < 1) nova.hp = 1;
    nova.energia = nova.energiaMax;
    nova.modDefesa = 0;
    nova.xp = 0;

    int destino = equipeAdicionar(&g->equipe, &nova);
    if (destino == 0) return RESS_ERRO;

    g->jogador.conquistadas++;
    g->jogador.pontuacao += pontosPorConquista(&nova);
    return destino == 1 ? RESS_SUCESSO_EQUIPE : RESS_SUCESSO_ARMAZENAMENTO;
}
