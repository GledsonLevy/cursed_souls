#ifndef EQUIPE_H
#define EQUIPE_H

#include "entidades.h"

#define MAX_EQUIPE 3
#define MAX_ARMAZENAMENTO 200

typedef struct {
    Criatura ativos[MAX_EQUIPE];
    int qtdAtivos;
    int indiceAtivo;
    Criatura *armazenamento;          /* vetor dinâmico (malloc/realloc) */
    int qtdArmazenamento;
    int capacidadeArmazenamento;
} Equipe;

void equipeInicializar(Equipe *e);
void equipeLiberar(Equipe *e);
/* retorna 1 = entrou na equipe, 2 = foi para o armazenamento, 0 = erro */
int equipeAdicionar(Equipe *e, const Criatura *c);
Criatura *equipeAtiva(Equipe *e);
const Criatura *equipeObterAtiva(const Equipe *e);
int equipeTrocarAtivo(Equipe *e, int indice);
int equipeTemVivo(const Equipe *e);
int equipeGarantirAtivoVivo(Equipe *e);
int equipeTrocarComArmazenamento(Equipe *e, int slotEquipe, int indiceArmazenamento);
void equipeCurarTodos(Equipe *e);
void equipeLimparModificadores(Equipe *e);

#endif
