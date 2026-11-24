#ifndef REGRAS_H
#define REGRAS_H

#include "entidades.h"
#include "equipe.h"

#define POCOES_INICIAIS 3
#define CORRENTES_INICIAIS 3
#define BONUS_PONTOS_CHEFE 500

typedef struct {
    Jogador jogador;
    Equipe equipe;
} EstadoJogo;

void estadoInicializar(EstadoJogo *g);
int estadoNovo(EstadoJogo *g, const char *nome, int especieInicial);
void estadoLiberar(EstadoJogo *g);

int calcularDano(const Criatura *atq, const Criatura *def, const Habilidade *h, int variacaoPct);
int xpParaProximoNivel(int nivel);
int ganharExperiencia(Criatura *c, int xp);          /* retorna níveis ganhos */
int xpPorDerrota(const Criatura *inimigo);
int pontosPorDerrota(const Criatura *inimigo);
int pontosPorConquista(const Criatura *c);
int validarNome(const char *nome);
int limitar(int valor, int minimo, int maximo);

#endif
