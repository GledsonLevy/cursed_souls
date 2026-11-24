#ifndef BATALHA_H
#define BATALHA_H

#include "regras.h"

#define BATALHA_LINHAS 6
#define BATALHA_LINHA_TAM 96

typedef enum { ACAO_ATACAR, ACAO_HABILIDADE, ACAO_TROCAR, ACAO_ITEM, ACAO_FUGIR } AcaoBatalha;
typedef enum { BAT_EM_ANDAMENTO, BAT_VITORIA, BAT_DERROTA, BAT_FUGA } EstadoBatalha;

typedef struct {
    Criatura inimigo;                    /* criatura inimiga que está em campo agora */
    int ehChefe;                         /* 1 = batalha contra um invocador (não permite fugir) */
    char treinador[MAX_NOME];            /* nome do invocador (vazio em batalha selvagem) */
    Criatura timeInimigo[MAX_EQUIPE];    /* equipe do invocador */
    int qtdTimeInimigo;
    int indiceInimigo;                   /* posição em timeInimigo da criatura em campo */
    EstadoBatalha estado;
    int precisaTrocar;                   /* ativa caiu: jogador deve escolher outra */
    int turno;
    int xpGanho, pontosGanhos;
    int qtdLog;
    char log[BATALHA_LINHAS][BATALHA_LINHA_TAM];
} Batalha;

void batalhaIniciar(Batalha *b, const Criatura *inimigo);                 /* criatura selvagem */
void batalhaIniciarTreinador(Batalha *b, const char *nome, const Criatura *time, int qtd);
/* retorna 1 se o turno foi consumido, 0 se a ação era inválida */
int batalhaExecutarTurno(Batalha *b, EstadoJogo *g, AcaoBatalha acao, int parametro);
int batalhaTrocaForcada(Batalha *b, EstadoJogo *g, int indice);

#endif
