#include "mapa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *NOMES_REGIAO[TOTAL_REGIOES] = {
    "Floresta Corrompida", "Ruínas Amaldiçoadas", "Covil do Guardião"
};

static void retangulo(Mapa *m, int x0, int y0, int x1, int y1, char c)
{
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            if (x >= 0 && x < m->largura && y >= 0 && y < m->altura) {
                m->tiles[y][x] = c;
            }
        }
    }
}

static void definirEspecies(Mapa *m, int nMin, int nMax, int qtd, const int *lista)
{
    m->nivelMin = nMin;
    m->nivelMax = nMax;
    m->qtdEspecies = qtd;
    for (int i = 0; i < qtd && i < MAX_ESPECIES_REGIAO; i++) m->especies[i] = lista[i];
}

static void montarRegiao(Mapa *m, int regiao)
{
    switch (regiao) {
    case 0: {
        static const int esp[] = {0, 0, 0, 1, 1, 1, 2, 2};
        definirEspecies(m, 2, 5, 8, esp);
        retangulo(m, 3, 2, 9, 5, ',');
        retangulo(m, 14, 2, 21, 5, ',');
        retangulo(m, 3, 9, 9, 12, ',');
        retangulo(m, 15, 9, 21, 12, ',');
        retangulo(m, 11, 1, 11, 5, 'T');
        retangulo(m, 13, 9, 13, 12, 'T');
        retangulo(m, 2, 1, 2, 3, 'T');
        retangulo(m, 22, 13, 22, 13, 'T');
        m->tiles[7][12] = 'F';
        m->tiles[7][MAPA_LARGURA - 2] = '>';
        break;
    }
    case 1: {
        static const int esp[] = {3, 3, 4, 4, 4, 5, 5, 6};
        definirEspecies(m, 5, 9, 8, esp);
        retangulo(m, 2, 2, 8, 5, ',');
        retangulo(m, 2, 9, 8, 12, ',');
        retangulo(m, 12, 2, 22, 5, ',');
        retangulo(m, 12, 9, 22, 12, ',');
        retangulo(m, 10, 1, 10, 5, '#');
        retangulo(m, 10, 9, 10, 13, '#');
        retangulo(m, 15, 3, 15, 4, '#');
        retangulo(m, 19, 10, 19, 11, '#');
        m->tiles[7][1] = '<';
        m->tiles[7][12] = 'F';
        m->tiles[7][MAPA_LARGURA - 2] = '>';
        break;
    }
    default: {
        definirEspecies(m, 10, 10, 0, NULL);   /* sem encontros aleatórios */
        retangulo(m, 5, 3, 6, 4, '#');
        retangulo(m, 5, 10, 6, 11, '#');
        retangulo(m, 18, 3, 19, 4, '#');
        retangulo(m, 18, 10, 19, 11, '#');
        retangulo(m, 10, 2, 10, 4, '#');
        retangulo(m, 14, 2, 14, 4, '#');
        m->tiles[7][1] = '<';
        m->tiles[7][4] = 'F';
        m->tiles[3][12] = 'B';
        break;
    }
    }
}

void mapaLiberar(Mapa *m)
{
    if (m == NULL) return;
    if (m->tiles != NULL) {
        for (int y = 0; y < m->altura; y++) free(m->tiles[y]);
        free(m->tiles);
    }
    free(m);
}

Mapa *mapaCriar(int regiao)
{
    if (regiao < 0 || regiao >= TOTAL_REGIOES) return NULL;

    Mapa *m = calloc(1, sizeof *m);
    if (m == NULL) return NULL;
    m->regiao = regiao;
    m->largura = MAPA_LARGURA;
    m->altura = MAPA_ALTURA;
    snprintf(m->nome, sizeof m->nome, "%s", NOMES_REGIAO[regiao]);

    m->tiles = calloc((size_t)m->altura, sizeof *m->tiles);
    if (m->tiles == NULL) {
        free(m);
        return NULL;
    }
    for (int y = 0; y < m->altura; y++) {
        m->tiles[y] = malloc((size_t)m->largura + 1);
        if (m->tiles[y] == NULL) {
            mapaLiberar(m);          /* libera as linhas já alocadas */
            return NULL;
        }
        memset(m->tiles[y], '.', (size_t)m->largura);
        m->tiles[y][m->largura] = '\0';
    }

    retangulo(m, 0, 0, m->largura - 1, 0, '#');
    retangulo(m, 0, m->altura - 1, m->largura - 1, m->altura - 1, '#');
    retangulo(m, 0, 0, 0, m->altura - 1, '#');
    retangulo(m, m->largura - 1, 0, m->largura - 1, m->altura - 1, '#');

    montarRegiao(m, regiao);
    return m;
}

char mapaTile(const Mapa *m, int x, int y)
{
    if (m == NULL || m->tiles == NULL) return '#';
    if (x < 0 || y < 0 || x >= m->largura || y >= m->altura) return '#';
    return m->tiles[y][x];
}

int mapaCaminhavel(const Mapa *m, int x, int y)
{
    char t = mapaTile(m, x, y);
    return t != '#' && t != 'T';
}

int mapaGerarCriatura(const Mapa *m, Criatura *saida)
{
    if (m == NULL || saida == NULL || m->qtdEspecies <= 0) return 0;
    int especie = m->especies[rand() % m->qtdEspecies];
    int nivel = m->nivelMin + rand() % (m->nivelMax - m->nivelMin + 1);
    return criaturaCriar(saida, especie, nivel);
}
