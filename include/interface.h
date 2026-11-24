#ifndef INTERFACE_H
#define INTERFACE_H

#include "raylib.h"
#include "batalha.h"
#include "mapa.h"
#include "pontuacao.h"
#include "regras.h"

#define JANELA_LARGURA 800
#define JANELA_ALTURA 600

typedef enum { BM_ACOES, BM_TROCA, BM_TROCA_FORCADA } ModoBatalha;

void interfaceMenu(int sel, const char *aviso);
void interfaceInstrucoes(void);
void interfaceRanking(const RegistroPontuacao *v, int n);
void interfaceNome(const char *titulo, const char *nome, const char *erro);
void interfaceInicial(int sel);
void interfaceExploracao(const Mapa *m, const EstadoJogo *g, const char *mensagem);
void interfaceBatalha(const Batalha *b, const EstadoJogo *g, ModoBatalha modo, int selAcao, int selTroca);
/* Transição ao encontrar uma criatura ou o chefe: tempo em segundos desde o início. */
void interfaceEncontro(const Mapa *m, const EstadoJogo *g, const Batalha *b, double tempo, int chefe);
double interfaceEncontroDuracao(int chefe);
void interfaceLore(int indice, int total, const char *texto);
/* resultado: -1 aguardando, 0 falha, 1 equipe, 2 armazenamento */
void interfaceRessurreicao(const Criatura *c, int chance, int correntes, int resultado, double tempo, const char *aviso);
void interfaceEquipe(const EstadoJogo *g, int sel, int marcado, const char *msg);
void interfacePausa(int sel, const char *msg);
void interfaceFim(int vitoria, const EstadoJogo *g, int sel, int temSave);

#endif
