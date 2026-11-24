#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"

/* Carrega os sprites da pasta assets/sprites/. Chamar depois de InitWindow().
 * Se um arquivo não existir, o jogo continua e desenha aquele item com formas simples. */
void assetsCarregar(void);
void assetsLiberar(void);

/* Cada função devolve NULL quando o sprite não foi carregado. */
const Texture2D *assetsCriatura(int especieId);   /* especie_0.png ... especie_6.png */
/* Jogador no mapa: jogador_baixo/cima/esquerda/direita.png. Cada imagem tem 32 de altura e pode ter
 * vários quadros lado a lado (ex.: 96x32 = 3 quadros). Sem elas, usa jogador.png (32x32). */
const Texture2D *assetsJogadorMapa(int direcao);
const Texture2D *assetsGuardiaoMapa(void);        /* guardiao_mapa.png (32x32) */
const Texture2D *assetsJogadorBatalha(void);      /* jogador_batalha.png (64x64) */
const Texture2D *assetsGuardiaoBatalha(void);     /* guardiao_batalha.png (96x96) */

/* Opcionais: se a imagem não existir, a interface desenha com formas simples. */
const Texture2D *assetsTile(char tile, int regiao);   /* '#' parede, ',' grama, 'T' árvore */
const Texture2D *assetsTileAnimado(char tile);        /* 'F' fogueira, '>' e '<' portais: tiras de 32x32 lado a lado */
const Texture2D *assetsLogo(void);                    /* titulo.png (300x60, ampliado 2x) */
const Texture2D *assetsFundoMenu(void);               /* fundo_menu.png (400x300, ampliado 2x) */
const Texture2D *assetsFundoBatalha(int regiao);      /* fundo_batalha_floresta/ruinas/covil.png */

#endif
