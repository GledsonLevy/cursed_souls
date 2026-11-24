#include "equipe.h"
#include <stdlib.h>
#include <string.h>

void equipeInicializar(Equipe *e)
{
    if (e == NULL) return;
    memset(e, 0, sizeof *e);
}

void equipeLiberar(Equipe *e)
{
    if (e == NULL) return;
    free(e->armazenamento);
    e->armazenamento = NULL;
    e->qtdArmazenamento = 0;
    e->capacidadeArmazenamento = 0;
}

int equipeAdicionar(Equipe *e, const Criatura *c)
{
    if (e == NULL || c == NULL) return 0;

    if (e->qtdAtivos < MAX_EQUIPE) {
        e->ativos[e->qtdAtivos++] = *c;
        return 1;
    }
    if (e->qtdArmazenamento >= MAX_ARMAZENAMENTO) return 0;

    if (e->qtdArmazenamento == e->capacidadeArmazenamento) {
        int novaCap = e->capacidadeArmazenamento ? e->capacidadeArmazenamento * 2 : 4;
        if (novaCap > MAX_ARMAZENAMENTO) novaCap = MAX_ARMAZENAMENTO;
        Criatura *novo = realloc(e->armazenamento, (size_t)novaCap * sizeof *novo);
        if (novo == NULL) return 0;
        e->armazenamento = novo;
        e->capacidadeArmazenamento = novaCap;
    }
    e->armazenamento[e->qtdArmazenamento++] = *c;
    return 2;
}

Criatura *equipeAtiva(Equipe *e)
{
    if (e == NULL || e->qtdAtivos <= 0) return NULL;
    if (e->indiceAtivo < 0 || e->indiceAtivo >= e->qtdAtivos) return NULL;
    return &e->ativos[e->indiceAtivo];
}

const Criatura *equipeObterAtiva(const Equipe *e)
{
    if (e == NULL || e->qtdAtivos <= 0) return NULL;
    if (e->indiceAtivo < 0 || e->indiceAtivo >= e->qtdAtivos) return NULL;
    return &e->ativos[e->indiceAtivo];
}

int equipeTrocarAtivo(Equipe *e, int indice)
{
    if (e == NULL || indice < 0 || indice >= e->qtdAtivos) return 0;
    if (indice == e->indiceAtivo) return 0;
    if (!criaturaEstaViva(&e->ativos[indice])) return 0;
    e->indiceAtivo = indice;
    return 1;
}

int equipeTemVivo(const Equipe *e)
{
    if (e == NULL) return 0;
    for (int i = 0; i < e->qtdAtivos; i++) {
        if (criaturaEstaViva(&e->ativos[i])) return 1;
    }
    return 0;
}

int equipeGarantirAtivoVivo(Equipe *e)
{
    if (e == NULL) return 0;
    const Criatura *a = equipeObterAtiva(e);
    if (a != NULL && criaturaEstaViva(a)) return 1;
    for (int i = 0; i < e->qtdAtivos; i++) {
        if (criaturaEstaViva(&e->ativos[i])) {
            e->indiceAtivo = i;
            return 1;
        }
    }
    return 0;
}

int equipeTrocarComArmazenamento(Equipe *e, int slotEquipe, int indiceArmazenamento)
{
    if (e == NULL) return 0;
    if (slotEquipe < 0 || slotEquipe >= e->qtdAtivos) return 0;
    if (indiceArmazenamento < 0 || indiceArmazenamento >= e->qtdArmazenamento) return 0;
    Criatura tmp = e->ativos[slotEquipe];
    e->ativos[slotEquipe] = e->armazenamento[indiceArmazenamento];
    e->armazenamento[indiceArmazenamento] = tmp;
    return 1;
}

void equipeCurarTodos(Equipe *e)
{
    if (e == NULL) return;
    for (int i = 0; i < e->qtdAtivos; i++) criaturaRestaurar(&e->ativos[i]);
    for (int i = 0; i < e->qtdArmazenamento; i++) criaturaRestaurar(&e->armazenamento[i]);
}

void equipeLimparModificadores(Equipe *e)
{
    if (e == NULL) return;
    for (int i = 0; i < e->qtdAtivos; i++) e->ativos[i].modDefesa = 0;
    for (int i = 0; i < e->qtdArmazenamento; i++) e->armazenamento[i].modDefesa = 0;
}
