#ifndef PONTUACAO_H
#define PONTUACAO_H

#include "entidades.h"

#define MAX_RANKING 10

typedef struct {
    char nome[MAX_NOME];
    int pontos;
} RegistroPontuacao;

/* Retorna a quantidade de registros lidos (ordenados do maior para o menor). */
int pontuacaoCarregar(const char *caminho, RegistroPontuacao *vetor, int maximo);
/* Insere o resultado, mantém só o top 10 e regrava o arquivo. Retorna 1 se ok. */
int pontuacaoRegistrar(const char *caminho, const char *nome, int pontos);

#endif
