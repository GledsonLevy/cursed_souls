#ifndef MAPA_H
#define MAPA_H

#include "entidades.h"

#define MAPA_LARGURA 25
#define MAPA_ALTURA 15
#define TAMANHO_TILE 32
#define TOTAL_REGIOES 3
#define MAX_ESPECIES_REGIAO 8

#define SPAWN_X 2
#define SPAWN_Y 7
#define SPAWN_X_DIREITA (MAPA_LARGURA - 3)

/* Tiles: '#' parede, 'T' árvore, '.' chão, ',' zona de encontros,
 *        '>' próxima região, '<' região anterior, 'F' fogueira, 'B' chefe */
typedef struct {
    int regiao;
    char nome[40];
    int largura, altura;
    char **tiles;                        /* matriz alocada dinamicamente */
    int nivelMin, nivelMax;
    int qtdEspecies;
    int especies[MAX_ESPECIES_REGIAO];
} Mapa;

Mapa *mapaCriar(int regiao);
void mapaLiberar(Mapa *m);
char mapaTile(const Mapa *m, int x, int y);
int mapaCaminhavel(const Mapa *m, int x, int y);
int mapaGerarCriatura(const Mapa *m, Criatura *saida);

#endif
