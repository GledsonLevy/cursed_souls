#include "entidades.h"
#include <stdio.h>
#include <string.h>

#define CRESCIMENTO_PCT 12   /* % do valor base ganho por nível */

typedef struct {
    const char *nome;
    Afinidade afinidade;
    Raridade raridade;
    int hp, ataque, defesa, velocidade, energia;
    Habilidade habilidades[MAX_HABILIDADES];
} ModeloEspecie;

static const ModeloEspecie MODELOS[TOTAL_ESPECIES] = {
    {"Lobo Abissal", AFIN_SOMBRIA, RAR_COMUM, 32, 9, 4, 8, 6,
        {{"Mordida Sombria", 8, 0, EFEITO_NENHUM}, {"Rugido do Abismo", 2, 3, EFEITO_REDUZ_DEFESA}}},
    {"Raposa de Cinzas", AFIN_FOGO, RAR_COMUM, 28, 10, 3, 11, 6,
        {{"Garra Flamejante", 8, 0, EFEITO_NENHUM}, {"Labareda", 14, 4, EFEITO_NENHUM}}},
    {"Serpente Glacial", AFIN_GELO, RAR_INCOMUM, 36, 10, 5, 7, 7,
        {{"Presa Gélida", 8, 0, EFEITO_NENHUM}, {"Abraço Congelante", 8, 3, EFEITO_DRENAR}}},
    {"Espectro do Caos", AFIN_CAOS, RAR_INCOMUM, 30, 12, 3, 10, 7,
        {{"Toque Caótico", 9, 0, EFEITO_NENHUM}, {"Distorção", 5, 3, EFEITO_REDUZ_DEFESA}}},
    {"Corvo Ruinoso", AFIN_SOMBRIA, RAR_RARA, 38, 13, 6, 12, 8,
        {{"Bico Sombrio", 10, 0, EFEITO_NENHUM}, {"Tempestade de Penas", 18, 4, EFEITO_NENHUM}}},
    {"Salamandra Rubra", AFIN_FOGO, RAR_EPICA, 46, 15, 8, 9, 8,
        {{"Chicote de Brasa", 11, 0, EFEITO_NENHUM}, {"Erupção", 22, 5, EFEITO_NENHUM}}},
    {"Wendigo Gélido", AFIN_GELO, RAR_LENDARIA, 54, 17, 9, 10, 9,
        {{"Garra Congelante", 12, 0, EFEITO_NENHUM}, {"Fome Eterna", 14, 4, EFEITO_DRENAR}}}
};

/* Equipe do Guardião Corrompido: espécie e nível de cada criatura (edite aqui para balancear). */
typedef struct {
    int especieId;
    int nivel;
} MembroChefe;

static const MembroChefe EQUIPE_CHEFE[TAMANHO_EQUIPE_CHEFE] = {
    {3, 10},   /* Espectro do Caos */
    {4, 11},   /* Corvo Ruinoso */
    {6, 12}    /* Wendigo Gélido */
};

static const char *NOMES_AFINIDADE[AFIN_TOTAL] = {"Sombria", "Fogo", "Gelo", "Caos"};
static const char *NOMES_RARIDADE[RAR_TOTAL] = {"Comum", "Incomum", "Rara", "Épica", "Lendária"};

static int escalar(int base, int nivel)
{
    return base + (base * (nivel - 1) * CRESCIMENTO_PCT) / 100;
}

static void calcularAtributos(Criatura *c)
{
    const ModeloEspecie *m = &MODELOS[c->especieId];
    c->hpMax = escalar(m->hp, c->nivel);
    c->ataque = escalar(m->ataque, c->nivel);
    c->defesa = escalar(m->defesa, c->nivel);
    c->velocidade = escalar(m->velocidade, c->nivel);
    c->energiaMax = m->energia + c->nivel / 4;
}

int criaturaCriar(Criatura *c, int especieId, int nivel)
{
    if (c == NULL || especieId < 0 || especieId >= TOTAL_ESPECIES ||
        nivel < 1 || nivel > NIVEL_MAXIMO) {
        return 0;
    }
    const ModeloEspecie *m = &MODELOS[especieId];
    memset(c, 0, sizeof *c);
    c->especieId = especieId;
    snprintf(c->nome, sizeof c->nome, "%s", m->nome);
    c->afinidade = m->afinidade;
    c->raridade = m->raridade;
    c->nivel = nivel;
    calcularAtributos(c);
    c->hp = c->hpMax;
    c->energia = c->energiaMax;
    for (int i = 0; i < MAX_HABILIDADES; i++) {
        c->habilidades[i] = m->habilidades[i];
    }
    return 1;
}

int criaturaSubirNivel(Criatura *c)
{
    if (c == NULL || c->nivel >= NIVEL_MAXIMO) return 0;
    int hpAntigo = c->hpMax;
    int enAntiga = c->energiaMax;
    c->nivel++;
    calcularAtributos(c);
    c->hp += c->hpMax - hpAntigo;
    c->energia += c->energiaMax - enAntiga;
    return 1;
}

int criaturaEstaViva(const Criatura *c)
{
    return c != NULL && c->hp > 0;
}

int criaturaDefesaEfetiva(const Criatura *c)
{
    int d = c->defesa + c->modDefesa;
    return d < 0 ? 0 : d;
}

void criaturaRestaurar(Criatura *c)
{
    if (c == NULL) return;
    c->hp = c->hpMax;
    c->energia = c->energiaMax;
    c->modDefesa = 0;
}

/* Ciclo de vantagens: Fogo > Gelo > Caos > Sombria > Fogo */
int multiplicadorAfinidade(Afinidade atq, Afinidade def)
{
    if (atq < 0 || atq >= AFIN_TOTAL || def < 0 || def >= AFIN_TOTAL) return 100;
    if (def == (Afinidade)((atq + 1) % AFIN_TOTAL)) return 150;
    if (atq == (Afinidade)((def + 1) % AFIN_TOTAL)) return 67;
    return 100;
}

const char *nomeAfinidade(Afinidade a)
{
    return (a >= 0 && a < AFIN_TOTAL) ? NOMES_AFINIDADE[a] : "?";
}

const char *nomeRaridade(Raridade r)
{
    return (r >= 0 && r < RAR_TOTAL) ? NOMES_RARIDADE[r] : "?";
}

int chefeMontarEquipe(Criatura *saida, int max)
{
    int n = 0;
    if (saida == NULL || max <= 0) return 0;
    for (int i = 0; i < TAMANHO_EQUIPE_CHEFE && n < max; i++) {
        if (criaturaCriar(&saida[n], EQUIPE_CHEFE[i].especieId, EQUIPE_CHEFE[i].nivel)) n++;
    }
    return n;
}
