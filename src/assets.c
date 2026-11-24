#include "assets.h"
#include "entidades.h"
#include "mapa.h"
#include <stdio.h>

/* Texturas do jogo. Ficam só neste arquivo; o resto do código usa as funções abaixo. */
static Texture2D criaturas[TOTAL_ESPECIES];
static Texture2D jogadorMapa, guardiaoMapa, jogadorBatalha, guardiaoBatalha;
static Texture2D arvore, fogueira, portalProximo, portalAnterior, logo;
static Texture2D paredes[TOTAL_REGIOES], gramas[TOTAL_REGIOES];   /* um tile por região */
static Texture2D fundoMenu, fundosBatalha[TOTAL_REGIOES];
static Texture2D jogadorDir[4];          /* indexado por DIR_BAIXO, DIR_CIMA, DIR_ESQUERDA, DIR_DIREITA */

/* Carrega assets/sprites/<nome>. Devolve uma textura com id 0 se não achar o arquivo. */
static Texture2D carregar(const char *nome)
{
    Texture2D vazia = {0};
    char caminho[256];

    /* primeiro procura ao lado do executável; depois, na pasta atual */
    snprintf(caminho, sizeof caminho, "%sassets/sprites/%s", GetApplicationDirectory(), nome);
    if (!FileExists(caminho)) snprintf(caminho, sizeof caminho, "assets/sprites/%s", nome);
    if (!FileExists(caminho)) return vazia;

    Texture2D t = LoadTexture(caminho);
    if (t.id != 0) SetTextureFilter(t, TEXTURE_FILTER_POINT);   /* sem borrar os pixels */
    return t;
}

void assetsCarregar(void)
{
    char nome[32];
    for (int i = 0; i < TOTAL_ESPECIES; i++) {
        snprintf(nome, sizeof nome, "especie_%d.png", i);
        criaturas[i] = carregar(nome);
    }
    jogadorMapa = carregar("jogador.png");     /* usado se faltar a imagem da direção */
    arvore = carregar("arvore.png");
    fogueira = carregar("fogueira.png");
    portalProximo = carregar("portal_proximo.png");
    portalAnterior = carregar("portal_anterior.png");
    logo = carregar("titulo.png");
    paredes[0] = carregar("parede_floresta.png");
    paredes[1] = carregar("parede_ruinas.png");
    paredes[2] = carregar("parede_covil.png");
    gramas[0] = carregar("grama_floresta.png");
    gramas[1] = carregar("grama_ruinas.png");          /* o Covil não tem grama */
    fundoMenu = carregar("fundo_menu.png");
    fundosBatalha[0] = carregar("fundo_batalha_floresta.png");
    fundosBatalha[1] = carregar("fundo_batalha_ruinas.png");
    fundosBatalha[2] = carregar("fundo_batalha_covil.png");
    jogadorDir[DIR_BAIXO] = carregar("jogador_baixo.png");
    jogadorDir[DIR_CIMA] = carregar("jogador_cima.png");
    jogadorDir[DIR_ESQUERDA] = carregar("jogador_esquerda.png");
    jogadorDir[DIR_DIREITA] = carregar("jogador_direita.png");
    guardiaoMapa = carregar("guardiao_mapa.png");
    jogadorBatalha = carregar("jogador_batalha.png");
    guardiaoBatalha = carregar("guardiao_batalha.png");
}

static void descarregar(Texture2D *t)
{
    if (t->id != 0) UnloadTexture(*t);
    t->id = 0;
}

void assetsLiberar(void)
{
    for (int i = 0; i < TOTAL_ESPECIES; i++) descarregar(&criaturas[i]);
    descarregar(&jogadorMapa);
    for (int i = 0; i < 4; i++) descarregar(&jogadorDir[i]);
    descarregar(&arvore);
    descarregar(&fogueira);
    descarregar(&portalProximo);
    descarregar(&portalAnterior);
    descarregar(&logo);
    descarregar(&fundoMenu);
    for (int i = 0; i < TOTAL_REGIOES; i++) {
        descarregar(&paredes[i]);
        descarregar(&gramas[i]);
        descarregar(&fundosBatalha[i]);
    }
    descarregar(&guardiaoMapa);
    descarregar(&jogadorBatalha);
    descarregar(&guardiaoBatalha);
}

const Texture2D *assetsCriatura(int especieId)
{
    if (especieId < 0 || especieId >= TOTAL_ESPECIES) return NULL;
    return criaturas[especieId].id != 0 ? &criaturas[especieId] : NULL;
}

const Texture2D *assetsJogadorMapa(int direcao)
{
    if (direcao >= 0 && direcao < 4 && jogadorDir[direcao].id != 0) return &jogadorDir[direcao];
    return jogadorMapa.id != 0 ? &jogadorMapa : NULL;
}
const Texture2D *assetsGuardiaoMapa(void) { return guardiaoMapa.id != 0 ? &guardiaoMapa : NULL; }
const Texture2D *assetsJogadorBatalha(void) { return jogadorBatalha.id != 0 ? &jogadorBatalha : NULL; }
const Texture2D *assetsGuardiaoBatalha(void) { return guardiaoBatalha.id != 0 ? &guardiaoBatalha : NULL; }

const Texture2D *assetsTile(char tile, int regiao)
{
    if (tile == 'T') return arvore.id != 0 ? &arvore : NULL;
    if (regiao < 0 || regiao >= TOTAL_REGIOES) return NULL;
    if (tile == '#') return paredes[regiao].id != 0 ? &paredes[regiao] : NULL;
    if (tile == ',') return gramas[regiao].id != 0 ? &gramas[regiao] : NULL;
    return NULL;
}

const Texture2D *assetsFundoMenu(void)
{
    return fundoMenu.id != 0 ? &fundoMenu : NULL;
}

const Texture2D *assetsFundoBatalha(int regiao)
{
    if (regiao < 0 || regiao >= TOTAL_REGIOES) return NULL;
    return fundosBatalha[regiao].id != 0 ? &fundosBatalha[regiao] : NULL;
}

const Texture2D *assetsTileAnimado(char tile)
{
    if (tile == 'F') return fogueira.id != 0 ? &fogueira : NULL;
    if (tile == '>') return portalProximo.id != 0 ? &portalProximo : NULL;
    if (tile == '<') return portalAnterior.id != 0 ? &portalAnterior : NULL;
    return NULL;
}

const Texture2D *assetsLogo(void)
{
    return logo.id != 0 ? &logo : NULL;
}
