#include "interface.h"
#include "assets.h"
#include "lore.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static const Color COR_FUNDO = {14, 10, 24, 255};
static const Color COR_PAINEL = {28, 20, 46, 240};
static const Color COR_BORDA = {120, 80, 190, 255};
static const Color COR_TEXTO = {230, 225, 240, 255};
static const Color COR_DESTAQUE = {255, 210, 90, 255};
static const Color COR_SUAVE = {150, 140, 170, 255};
static const Color COR_MANTO_JOGADOR = {40, 100, 110, 255};
static const Color COR_BRILHO_JOGADOR = {170, 120, 240, 255};
static const Color COR_MANTO_CHEFE = {60, 24, 70, 255};
static const Color COR_BRILHO_CHEFE = {200, 60, 130, 255};

/* ---------- utilitários ---------- */

/* As imagens de fundo trazem o nome do arquivo escrito nas primeiras linhas; essas linhas são cortadas. */
#define FUNDO_CORTE_TOPO 22

/* Fundo de tela cheia, ampliado em número inteiro (400x300 vira 2x). O que sobra embaixo fica na cor "reserva".
 * Sem imagem, a tela inteira usa a cor. */
static void fundoTela(const Texture2D *tex, Color reserva)
{
    ClearBackground(reserva);
    if (tex == NULL) return;
    int escala = JANELA_LARGURA / tex->width;
    float altura = (float)(tex->height - FUNDO_CORTE_TOPO);
    Rectangle origem = {0.0f, (float)FUNDO_CORTE_TOPO, (float)tex->width, altura};
    Rectangle destino = {0.0f, 0.0f, (float)(tex->width * escala), altura * (float)escala};
    Vector2 zero = {0.0f, 0.0f};
    DrawTexturePro(*tex, origem, destino, zero, 0.0f, WHITE);
}

static void textoCentro(const char *s, int cx, int y, int tam, Color c)
{
    DrawText(s, cx - MeasureText(s, tam) / 2, y, tam, c);
}

static void painel(int x, int y, int w, int h)
{
    DrawRectangle(x, y, w, h, COR_PAINEL);
    DrawRectangleLines(x, y, w, h, COR_BORDA);
}

static Color corHp(int atual, int max)
{
    float r = max > 0 ? (float)atual / (float)max : 0.0f;
    if (r > 0.5f) return (Color){90, 200, 100, 255};
    if (r > 0.2f) return (Color){230, 200, 70, 255};
    return (Color){220, 70, 70, 255};
}

static void barra(int x, int y, int w, int h, int atual, int max, Color cor)
{
    DrawRectangle(x, y, w, h, (Color){40, 30, 60, 255});
    if (max > 0 && atual > 0) DrawRectangle(x, y, w * atual / max, h, cor);
    DrawRectangleLines(x, y, w, h, BLACK);
}

static Color corAfinidade(Afinidade a)
{
    switch (a) {
    case AFIN_SOMBRIA: return (Color){120, 70, 170, 255};
    case AFIN_FOGO: return (Color){220, 100, 40, 255};
    case AFIN_GELO: return (Color){100, 180, 230, 255};
    default: return (Color){200, 60, 130, 255};
    }
}

static Color corRaridade(Raridade r)
{
    switch (r) {
    case RAR_COMUM: return (Color){170, 170, 170, 255};
    case RAR_INCOMUM: return (Color){80, 200, 120, 255};
    case RAR_RARA: return (Color){80, 140, 255, 255};
    case RAR_EPICA: return (Color){190, 90, 255, 255};
    default: return (Color){255, 200, 60, 255};
    }
}

static void opcoes(const char *const *ops, int n, int sel, int x, int y, int tam, int esp, int desativada)
{
    for (int i = 0; i < n; i++) {
        Color c = (i == sel) ? COR_DESTAQUE : COR_TEXTO;
        if (i == desativada) c = COR_SUAVE;
        if (i == sel) DrawText(">", x - tam - 4, y + i * esp, tam, c);
        DrawText(ops[i], x, y + i * esp, tam, c);
    }
}

/* Reserva: criatura desenhada com formas simples, usada quando não há sprite. */
static void desenharCriaturaForma(int cx, int cy, float esc, const Criatura *c, float alfa)
{
    Color cor = Fade(corAfinidade(c->afinidade), alfa);
    Color aura = corRaridade(c->raridade);
    int id = c->especieId;
    float t = (float)GetTime();
    int w = (int)((44 + (id % 4) * 8) * esc);
    int h = (int)((32 + (id % 3) * 8) * esc);
    cy += (int)(sinf(t * 2.0f + (float)id) * 4.0f * esc);

    DrawEllipse(cx, cy + h + (int)(8 * esc), (float)w * 0.9f, 8.0f * esc, Fade(BLACK, 0.5f * alfa));
    for (int i = 4; i >= 1; i--) {
        DrawCircle(cx, cy, (float)w * (0.9f + 0.18f * (float)i), Fade(aura, 0.07f * alfa));
    }
    int espinhos = 2 + id % 4;
    for (int i = 0; i < espinhos; i++) {
        int ex = cx - w / 2 + i * (w / espinhos);
        DrawRectangle(ex, cy - h - (int)(10 * esc), (int)(8 * esc), (int)(14 * esc), Fade(aura, alfa));
    }
    DrawEllipse(cx, cy, (float)w, (float)h, cor);
    DrawEllipseLines(cx, cy, (float)w, (float)h, Fade(BLACK, alfa));
    DrawCircle(cx - w / 3, cy - h / 4, 6.0f * esc, Fade(RAYWHITE, alfa));
    DrawCircle(cx + w / 3, cy - h / 4, 6.0f * esc, Fade(RAYWHITE, alfa));
    DrawCircle(cx - w / 3, cy - h / 4, 3.0f * esc, Fade((Color){200, 20, 20, 255}, alfa));
    DrawCircle(cx + w / 3, cy - h / 4, 3.0f * esc, Fade((Color){200, 20, 20, 255}, alfa));
    DrawRectangle(cx - w / 4, cy + h / 3, w / 2, (int)(3 * esc), Fade(BLACK, alfa));
}

/* Invocador (jogador e Guardião): figura encapuzada; só mudam as cores. */
static void desenharInvocador(int cx, int cy, float esc, Color manto, Color brilho, int correntes)
{
    float t = (float)GetTime();
    DrawEllipse(cx, cy + (int)(24 * esc), 14.0f * esc, 5.0f * esc, Fade(BLACK, 0.5f));
    DrawEllipse(cx, cy + (int)(6 * esc), 12.0f * esc, 18.0f * esc, manto);
    DrawEllipseLines(cx, cy + (int)(6 * esc), 12.0f * esc, 18.0f * esc, BLACK);
    DrawCircle(cx, cy - (int)(12 * esc), 10.0f * esc, manto);
    DrawCircle(cx, cy - (int)(11 * esc), 6.0f * esc, (Color){8, 4, 14, 255});
    DrawCircle(cx - (int)(3 * esc), cy - (int)(11 * esc), 1.6f * esc, brilho);
    DrawCircle(cx + (int)(3 * esc), cy - (int)(11 * esc), 1.6f * esc, brilho);
    DrawCircle(cx, cy + (int)(2 * esc), 3.0f * esc, brilho);
    if (correntes) {
        for (int i = 0; i < 4; i++) {
            float ang = t * 1.5f + (float)i * 1.57f;
            DrawCircle(cx + (int)(cosf(ang) * 26.0f * esc), cy + (int)(sinf(ang) * 10.0f * esc) + (int)(4 * esc),
                       2.5f * esc, Fade(brilho, 0.8f));
        }
    }
}

/* Desenha um texto que tem "\n" em várias linhas. */
static void textoEmLinhas(const char *texto, int x, int y, int tam, int espac, Color cor)
{
    char linha[100];
    int n = 0, i = 0;
    for (const char *p = texto; ; p++) {
        if (*p == '\n' || *p == '\0') {
            linha[n] = '\0';
            DrawText(linha, x, y + i * espac, tam, cor);
            i++;
            n = 0;
            if (*p == '\0') break;
        } else if (n < (int)sizeof linha - 1) {
            linha[n++] = *p;
        }
    }
}

/* Desenha a criatura com o sprite (se existir) ou com a forma simples.
 * espelhar = 1 vira o sprite para a esquerda: o inimigo fica de frente para o jogador. */
static void desenharCriaturaDir(int cx, int cy, float esc, const Criatura *c, float alfa, int espelhar)
{
    const Texture2D *tex = assetsCriatura(c->especieId);
    if (tex == NULL) {
        desenharCriaturaForma(cx, cy, esc, c, alfa);
        return;
    }
    int escala = (int)(esc * 1.5f + 0.5f);            /* ampliação inteira: pixels nítidos */
    if (escala < 1) escala = 1;
    float lado = (float)(tex->width * escala);
    cy += (int)(sinf((float)GetTime() * 2.0f + (float)c->especieId) * 3.0f);

    Color aura = corRaridade(c->raridade);
    for (int i = 3; i >= 1; i--) {
        DrawCircle(cx, cy, lado * (0.35f + 0.1f * (float)i), Fade(aura, 0.06f * alfa));
    }
    DrawEllipse(cx, cy + (int)(lado * 0.45f), lado * 0.3f, lado * 0.05f, Fade(BLACK, 0.5f * alfa));

    Rectangle origem = {0.0f, 0.0f, (float)tex->width * (espelhar ? -1.0f : 1.0f), (float)tex->height};
    Rectangle destino = {(float)cx - lado / 2.0f, (float)cy - lado / 2.0f, lado, lado};
    Vector2 zero = {0.0f, 0.0f};
    DrawTexturePro(*tex, origem, destino, zero, 0.0f, Fade(WHITE, alfa));
}

static void desenharCriatura(int cx, int cy, float esc, const Criatura *c, float alfa)
{
    desenharCriaturaDir(cx, cy, esc, c, alfa, 0);
}

static void infoCriatura(int x, int y, int w, const Criatura *c, int mostrarHp)
{
    painel(x, y, w, 62);
    DrawText(TextFormat("%s  Nv %d", c->nome, c->nivel), x + 8, y + 6, 18, COR_TEXTO);
    DrawText(nomeAfinidade(c->afinidade), x + 8, y + 26, 12, corAfinidade(c->afinidade));
    DrawText(nomeRaridade(c->raridade), x + 70, y + 26, 12, corRaridade(c->raridade));
    barra(x + 8, y + 42, w - 16, 12, c->hp, c->hpMax, corHp(c->hp, c->hpMax));
    if (mostrarHp) {
        DrawText(TextFormat("%d/%d  EN %d/%d", c->hp, c->hpMax, c->energia, c->energiaMax),
                 x + w - 130, y + 26, 12, COR_SUAVE);
    }
}

/* ---------- telas ---------- */

void interfaceMenu(int sel, const char *aviso)
{
    static const char *ops[] = {"Novo Jogo", "Continuar", "Instruções", "Ranking", "Sair"};
    fundoTela(assetsFundoMenu(), COR_FUNDO);
    for (int i = 0; i < 24; i++) {
        float t = (float)GetTime() * 0.6f + (float)i * 1.7f;
        DrawCircle(60 + (i * 83) % 740, 600 - (int)fmodf(t * 40.0f + (float)(i * 37), 620.0f),
                   2.0f + (float)(i % 3), Fade(COR_BORDA, 0.5f));
    }
    const Texture2D *logo = assetsLogo();
    if (logo != NULL) {
        Rectangle origem = {0.0f, 0.0f, (float)logo->width, (float)logo->height};
        Rectangle destino = {(float)((JANELA_LARGURA - logo->width * 2) / 2), 55.0f,
                             (float)(logo->width * 2), (float)(logo->height * 2)};     /* ampliado 2x */
        Vector2 zero = {0.0f, 0.0f};
        DrawTexturePro(*logo, origem, destino, zero, 0.0f, WHITE);
        textoCentro("Ecos Amaldiçoados", JANELA_LARGURA / 2, 190, 28, COR_TEXTO);
    } else {
        textoCentro("CURSED SOULS", JANELA_LARGURA / 2, 90, 64, COR_DESTAQUE);
        textoCentro("Ecos Amaldiçoados", JANELA_LARGURA / 2, 160, 28, COR_TEXTO);
    }
    int x = JANELA_LARGURA / 2 - 80;
    opcoes(ops, 5, sel, x, 250, 28, 44, -1);
    if (aviso != NULL && aviso[0] != '\0') textoCentro(aviso, JANELA_LARGURA / 2, 500, 18, (Color){230, 120, 120, 255});
    textoCentro("Derrote. Ressuscite. Conquiste.", JANELA_LARGURA / 2, 550, 18, COR_TEXTO);
}

void interfaceInstrucoes(void)
{
    ClearBackground(COR_FUNDO);
    textoCentro("INSTRUÇÕES", JANELA_LARGURA / 2, 30, 36, COR_DESTAQUE);
    static const char *linhas[] = {
        "Explore as regiões, derrote criaturas selvagens e ressuscite suas essências.",
        "",
        "EXPLORAÇÃO:  setas ou WASD movem | E equipe | ESC pausa",
        "  Grama escura = encontros | F = fogueira (cura tudo e repõe poções)",
        "  > e < = trocar de região | o Guardião, no Covil, é o chefe (um invocador)",
        "",
        "BATALHA:  setas escolhem | ENTER confirma | ESC volta (na troca)",
        "  Atacar (grátis) | Habilidade (gasta energia) | Trocar | Poção | Fugir",
        "  Afinidades: Fogo > Gelo > Caos > Sombria > Fogo (dano x1,5 / x0,67)",
        "",
        "RESSURREIÇÃO:  após vencer, ENTER tenta reviver a criatura.",
        "  C usa uma carga de Correntes da Maldição (+20% de chance).",
        "  Comum 75% | Incomum 55% | Rara 35% | Épica 20% | Lendária 10%",
        "",
        "OBJETIVO: derrote o Guardião Corrompido e as 3 criaturas dele. Equipe caída = derrota.",
        "Salve pela pausa (ESC). Em Continuar, digite o nome do jogador do save.",
        "O ranking guarda a melhor pontuação de cada nome."
    };
    for (int i = 0; i < (int)(sizeof linhas / sizeof linhas[0]); i++) {
        DrawText(linhas[i], 40, 90 + i * 26, 18, COR_TEXTO);
    }
    textoCentro("ESC ou ENTER para voltar", JANELA_LARGURA / 2, 560, 18, COR_SUAVE);
}

void interfaceRanking(const RegistroPontuacao *v, int n)
{
    fundoTela(assetsFundoMenu(), COR_FUNDO);
    textoCentro("RANKING", JANELA_LARGURA / 2, 40, 40, COR_DESTAQUE);
    if (n == 0) textoCentro("Nenhuma partida registrada ainda.", JANELA_LARGURA / 2, 250, 22, COR_SUAVE);
    for (int i = 0; i < n; i++) {
        Color c = i == 0 ? COR_DESTAQUE : COR_TEXTO;
        DrawText(TextFormat("%2d.  %s", i + 1, v[i].nome), 220, 120 + i * 38, 26, c);
        DrawText(TextFormat("%d", v[i].pontos), 560, 120 + i * 38, 26, c);
    }
    textoCentro("ESC ou ENTER para voltar", JANELA_LARGURA / 2, 560, 18, COR_SUAVE);
}

void interfaceNome(const char *titulo, const char *nome, const char *erro)
{
    fundoTela(assetsFundoMenu(), COR_FUNDO);
    textoCentro(titulo, JANELA_LARGURA / 2, 190, 32, COR_TEXTO);
    painel(JANELA_LARGURA / 2 - 200, 260, 400, 50);
    const char *cursor = ((int)(GetTime() * 2.0) % 2) ? "_" : "";
    DrawText(TextFormat("%s%s", nome, cursor), JANELA_LARGURA / 2 - 188, 272, 28, COR_DESTAQUE);
    textoCentro("Letras, números, _ e - (até 20 caracteres)", JANELA_LARGURA / 2, 330, 16, COR_SUAVE);
    if (erro != NULL && erro[0] != '\0') textoCentro(erro, JANELA_LARGURA / 2, 370, 18, (Color){230, 120, 120, 255});
    textoCentro("ENTER confirma | ESC volta", JANELA_LARGURA / 2, 540, 18, COR_SUAVE);
}

void interfaceInicial(int sel)
{
    ClearBackground(COR_FUNDO);
    textoCentro("Escolha a sua primeira essência", JANELA_LARGURA / 2, 40, 32, COR_DESTAQUE);
    for (int i = 0; i < 3; i++) {
        Criatura c;
        criaturaCriar(&c, i, 5);
        int x = 30 + i * 253;
        painel(x, 110, 237, 400);
        if (i == sel) DrawRectangleLines(x - 2, 108, 241, 404, COR_DESTAQUE);
        desenharCriatura(x + 118, 220, 1.4f, &c, 1.0f);
        textoCentro(c.nome, x + 118, 320, 20, COR_TEXTO);
        textoCentro(TextFormat("%s | %s", nomeAfinidade(c.afinidade), nomeRaridade(c.raridade)),
                    x + 118, 346, 14, corAfinidade(c.afinidade));
        DrawText(TextFormat("HP %d   ATQ %d", c.hpMax, c.ataque), x + 20, 380, 16, COR_TEXTO);
        DrawText(TextFormat("DEF %d   VEL %d", c.defesa, c.velocidade), x + 20, 402, 16, COR_TEXTO);
        DrawText(c.habilidades[0].nome, x + 20, 436, 14, COR_SUAVE);
        DrawText(TextFormat("%s (EN %d)", c.habilidades[1].nome, c.habilidades[1].custoEnergia),
                 x + 20, 456, 14, COR_SUAVE);
    }
    textoCentro("setas escolhem | ENTER confirma | ESC volta", JANELA_LARGURA / 2, 550, 18, COR_SUAVE);
}

static void desenharTile(const Mapa *m, int tx, int ty, char t, int chefeDerrotado)
{
    int x = tx * TAMANHO_TILE, y = ty * TAMANHO_TILE, s = TAMANHO_TILE;
    Color chao = (m->regiao == 0) ? (Color){26, 34, 28, 255}
               : (m->regiao == 1) ? (Color){38, 32, 44, 255} : (Color){40, 24, 30, 255};
    DrawRectangle(x, y, s, s, chao);
    const Texture2D *tex = assetsTile(t, m->regiao);   /* parede, grama e árvore: imagem por cima do chão */
    if (tex != NULL) {
        DrawTexture(*tex, x, y, WHITE);
        return;
    }
    const Texture2D *anim = assetsTileAnimado(t);       /* fogueira e portais: 3 quadros que se alternam */
    if (anim != NULL) {
        int quadros = anim->width / TAMANHO_TILE;
        if (quadros < 1) quadros = 1;
        int quadro = (int)(GetTime() * 6.0) % quadros;
        Rectangle origem = {(float)(quadro * TAMANHO_TILE), 0.0f, (float)TAMANHO_TILE, (float)anim->height};
        Vector2 pos = {(float)x, (float)y};
        DrawTextureRec(*anim, origem, pos, WHITE);
        return;
    }
    switch (t) {
    case '#':
        DrawRectangle(x, y, s, s, (Color){70, 62, 84, 255});
        DrawRectangleLines(x, y, s, s, (Color){40, 34, 52, 255});
        break;
    case 'T':
        DrawRectangle(x + 13, y + 18, 6, 14, (Color){70, 45, 30, 255});
        DrawCircle(x + 16, y + 14, 12.0f, (Color){28, 80, 50, 255});
        break;
    case ',':
        DrawRectangle(x, y, s, s, (Color){44, 30, 70, 255});
        DrawLine(x + 6, y + 24, x + 9, y + 12, (Color){110, 70, 170, 255});
        DrawLine(x + 18, y + 26, x + 21, y + 10, (Color){110, 70, 170, 255});
        DrawLine(x + 26, y + 22, x + 27, y + 14, (Color){110, 70, 170, 255});
        break;
    case '>':
    case '<':
        DrawRectangle(x + 4, y + 2, s - 8, s - 4, Fade((Color){80, 200, 230, 255}, 0.5f + 0.3f * sinf((float)GetTime() * 3.0f)));
        DrawText(t == '>' ? ">" : "<", x + 11, y + 6, 22, WHITE);
        break;
    case 'F':
        DrawCircle(x + 16, y + 18, 9.0f + sinf((float)GetTime() * 8.0f), (Color){240, 140, 40, 255});
        DrawCircle(x + 16, y + 20, 5.0f, (Color){255, 220, 100, 255});
        break;
    case 'B':
        if (!chefeDerrotado) {
            DrawCircle(x + 16, y + 18, 15.0f + sinf((float)GetTime() * 4.0f), Fade(COR_BRILHO_CHEFE, 0.25f));
            const Texture2D *tex = assetsGuardiaoMapa();
            if (tex != NULL) DrawTexture(*tex, x, y, WHITE);
            else desenharInvocador(x + 16, y + 14, 0.62f, COR_MANTO_CHEFE, COR_BRILHO_CHEFE, 1);
        }
        break;
    default:
        break;
    }
}

void interfaceExploracao(const Mapa *m, const EstadoJogo *g, const char *mensagem)
{
    ClearBackground(COR_FUNDO);
    for (int y = 0; y < m->altura; y++) {
        for (int x = 0; x < m->largura; x++) {
            desenharTile(m, x, y, m->tiles[y][x], g->jogador.chefeDerrotado);
        }
    }
    int px = g->jogador.x * TAMANHO_TILE + 16, py = g->jogador.y * TAMANHO_TILE + 16;
    const Texture2D *spriteJogador = assetsJogadorMapa(g->jogador.direcao);
    if (spriteJogador != NULL) {
        int quadros = spriteJogador->width / TAMANHO_TILE;          /* quadros lado a lado na imagem */
        if (quadros < 1) quadros = 1;
        Rectangle origem = {(float)((g->jogador.passo % quadros) * TAMANHO_TILE), 0.0f,
                            (float)TAMANHO_TILE, (float)spriteJogador->height};
        Vector2 pos = {(float)(g->jogador.x * TAMANHO_TILE), (float)(g->jogador.y * TAMANHO_TILE)};
        DrawTextureRec(*spriteJogador, origem, pos, WHITE);
    } else desenharInvocador(px, py - 2, 0.6f, COR_MANTO_JOGADOR, COR_BRILHO_JOGADOR, 0);

    int hy = MAPA_ALTURA * TAMANHO_TILE;
    painel(0, hy, JANELA_LARGURA, JANELA_ALTURA - hy);
    DrawText(TextFormat("%s  |  %s", g->jogador.nome, m->nome), 12, hy + 8, 18, COR_DESTAQUE);
    DrawText(TextFormat("Pontos: %d   Correntes: %d   Poções: %d   Conquistadas: %d",
                        g->jogador.pontuacao, g->jogador.correntes, g->jogador.pocoes,
                        g->jogador.conquistadas), 12, hy + 32, 16, COR_TEXTO);
    for (int i = 0; i < g->equipe.qtdAtivos; i++) {
        const Criatura *c = &g->equipe.ativos[i];
        int x = 12 + i * 262;
        DrawText(TextFormat("%s%s Nv%d", i == g->equipe.indiceAtivo ? "* " : "", c->nome, c->nivel),
                 x, hy + 60, 14, COR_TEXTO);
        barra(x, hy + 78, 240, 10, c->hp, c->hpMax, corHp(c->hp, c->hpMax));
    }
    if (mensagem != NULL && mensagem[0] != '\0') DrawText(mensagem, 12, hy + 96, 16, COR_DESTAQUE);
    DrawText("E: equipe | ESC: pausa", 600, hy + 8, 14, COR_SUAVE);
}

/* deslX desloca o inimigo para a direita (usado na animação de entrada). */
static void desenharBatalha(const Batalha *b, const EstadoJogo *g, ModoBatalha modo, int selAcao, int selTroca, int deslX)
{
    const Criatura *ativa = equipeObterAtiva(&g->equipe);
    const Texture2D *fundo = assetsFundoBatalha(g->jogador.regiao);
    fundoTela(fundo, (Color){20, 14, 34, 255});
    if (fundo == NULL) DrawRectangle(0, 290, JANELA_LARGURA, 10, (Color){40, 28, 64, 255});
    infoCriatura(40, 30, 320, &b->inimigo, 0);
    float alfaInimigo = criaturaEstaViva(&b->inimigo) ? 1.0f : 0.3f;
    if (b->ehChefe) {
        int restantes = b->qtdTimeInimigo - b->indiceInimigo - (criaturaEstaViva(&b->inimigo) ? 0 : 1);
        DrawRectangle(36, 4, 250, 22, Fade(BLACK, 0.6f));
        DrawRectangle(36, 96, 250, 22, Fade(BLACK, 0.6f));
        DrawText(TextFormat("%s (chefe)", b->treinador), 40, 8, 16, (Color){230, 80, 80, 255});
        for (int i = 0; i < b->qtdTimeInimigo; i++) {
            DrawCircle(52 + i * 20, 106, 7.0f, i < restantes ? COR_BRILHO_CHEFE : (Color){60, 40, 70, 255});
        }
        DrawText(TextFormat("%d criatura(s) restante(s)", restantes), 120, 99, 14, COR_SUAVE);
        const Texture2D *spriteChefe = assetsGuardiaoBatalha();
        if (spriteChefe != NULL) {
            /* o sprite olha para a direita; espelhado, ele olha para o jogador */
            Rectangle origem = {0.0f, 0.0f, -(float)spriteChefe->width, (float)spriteChefe->height};
            Rectangle destino = {(float)(600 + deslX), 20.0f,           /* ampliado 2x, como as criaturas */
                                 (float)(spriteChefe->width * 2), (float)(spriteChefe->height * 2)};
            Vector2 zero = {0.0f, 0.0f};
            DrawTexturePro(*spriteChefe, origem, destino, zero, 0.0f, WHITE);
        } else {
            desenharInvocador(735 + deslX, 118, 1.7f, COR_MANTO_CHEFE, COR_BRILHO_CHEFE, 1);
        }
        desenharCriaturaDir(500 + deslX, 150, 1.4f, &b->inimigo, alfaInimigo, 1);
    } else {
        desenharCriaturaDir(590 + deslX, 130, 1.4f, &b->inimigo, alfaInimigo, 1);
    }
    const Texture2D *spriteJogador = assetsJogadorBatalha();
    if (spriteJogador != NULL) {
        Rectangle origem = {0.0f, 0.0f, (float)spriteJogador->width, (float)spriteJogador->height};
        Rectangle destino = {10.0f, 186.0f, (float)(spriteJogador->width * 2), (float)(spriteJogador->height * 2)};
        Vector2 zero = {0.0f, 0.0f};
        DrawTexturePro(*spriteJogador, origem, destino, zero, 0.0f, WHITE);
    }
    if (ativa != NULL) {
        desenharCriatura(250, 250, 1.4f, ativa, criaturaEstaViva(ativa) ? 1.0f : 0.3f);
        infoCriatura(440, 220, 330, ativa, 1);
    }

    painel(10, 400, 430, 190);
    for (int i = 0; i < b->qtdLog; i++) DrawText(b->log[i], 20, 408 + i * 28, 16, COR_TEXTO);

    painel(450, 340, 340, 250);
    if (b->estado != BAT_EM_ANDAMENTO) {
        const char *t = b->estado == BAT_VITORIA ? "Vitória!" : b->estado == BAT_DERROTA ? "Derrota..." : "Fuga!";
        textoCentro(t, 620, 400, 36, COR_DESTAQUE);
        if (b->estado == BAT_VITORIA) {
            textoCentro(TextFormat("+%d XP   +%d pontos", b->xpGanho, b->pontosGanhos), 620, 460, 18, COR_TEXTO);
        }
        textoCentro("ENTER para continuar", 620, 540, 18, COR_SUAVE);
        return;
    }
    if (modo == BM_ACOES && ativa != NULL) {
        char a[64], h[80], p[40];
        snprintf(a, sizeof a, "Atacar: %s", ativa->habilidades[0].nome);
        snprintf(h, sizeof h, "Habilidade: %s (EN %d)", ativa->habilidades[1].nome, ativa->habilidades[1].custoEnergia);
        snprintf(p, sizeof p, "Poção (%d)", g->jogador.pocoes);
        const char *ops[] = {a, h, "Trocar criatura", p, "Fugir"};
        opcoes(ops, 5, selAcao, 482, 356, 18, 40, -1);
    } else {
        DrawText(modo == BM_TROCA_FORCADA ? "Escolha a próxima criatura:" : "Trocar para:", 466, 350, 18, COR_DESTAQUE);
        for (int i = 0; i < g->equipe.qtdAtivos; i++) {
            const Criatura *c = &g->equipe.ativos[i];
            int bloqueada = !criaturaEstaViva(c) || i == g->equipe.indiceAtivo;
            Color cor = i == selTroca ? COR_DESTAQUE : (bloqueada ? COR_SUAVE : COR_TEXTO);
            if (i == selTroca) DrawText(">", 466, 390 + i * 50, 18, cor);
            DrawText(TextFormat("%s Nv%d", c->nome, c->nivel), 486, 390 + i * 50, 18, cor);
            barra(486, 414 + i * 50, 200, 8, c->hp, c->hpMax, corHp(c->hp, c->hpMax));
        }
        if (modo == BM_TROCA) DrawText("ESC volta", 466, 560, 14, COR_SUAVE);
    }
}

void interfaceBatalha(const Batalha *b, const EstadoJogo *g, ModoBatalha modo, int selAcao, int selTroca)
{
    desenharBatalha(b, g, modo, selAcao, selTroca, 0);
}

void interfaceRessurreicao(const Criatura *c, int chance, int correntes, int resultado, double tempo, const char *aviso)
{
    ClearBackground((Color){12, 8, 22, 255});
    textoCentro("ESSÊNCIA LIBERTADA", JANELA_LARGURA / 2, 30, 34, COR_DESTAQUE);
    float t = (float)GetTime();
    float alfa = resultado == 0 ? 0.15f : 0.6f;
    if (resultado > 0 && tempo > 1.6) alfa = 1.0f;
    desenharCriatura(400, 230, 1.8f, c, alfa);
    for (int i = 0; i < 16; i++) {
        float sub = fmodf(t * 70.0f + (float)(i * 29), 200.0f);
        DrawCircle(400 + (int)(sinf(t * 2.0f + (float)i) * 70.0f), 330 - (int)sub, 3.0f,
                   Fade(corRaridade(c->raridade), 1.0f - sub / 200.0f));
    }
    textoCentro(TextFormat("%s  (Nv %d, %s)", c->nome, c->nivel, nomeRaridade(c->raridade)),
                JANELA_LARGURA / 2, 350, 22, COR_TEXTO);
    painel(150, 390, 500, 170);
    if (resultado == -1) {
        textoCentro(TextFormat("Chance de sucesso: %d%%", chance), JANELA_LARGURA / 2, 405, 24, COR_DESTAQUE);
        textoCentro("ENTER: tentar ressuscitar", JANELA_LARGURA / 2, 450, 20, COR_TEXTO);
        textoCentro(TextFormat("C: usar Correntes da Maldição (+%d%%)  [%d carga(s)]", 20, correntes),
                    JANELA_LARGURA / 2, 485, 18, correntes > 0 ? COR_TEXTO : COR_SUAVE);
        textoCentro("ESC: deixar a essência se dissipar", JANELA_LARGURA / 2, 520, 18, COR_SUAVE);
        if (aviso != NULL && aviso[0] != '\0') textoCentro(aviso, JANELA_LARGURA / 2, 372, 16, (Color){230, 120, 120, 255});
    } else if (tempo < 1.6) {
        textoCentro("A essência se agita...", JANELA_LARGURA / 2, 450, 26, COR_TEXTO);
    } else if (resultado == 0) {
        textoCentro("A essência se dissipou.", JANELA_LARGURA / 2, 430, 26, (Color){230, 120, 120, 255});
        textoCentro("ENTER para continuar", JANELA_LARGURA / 2, 520, 18, COR_SUAVE);
    } else {
        textoCentro("SUCESSO! Um novo Eco Amaldiçoado é seu.", JANELA_LARGURA / 2, 425, 24, (Color){120, 230, 140, 255});
        textoCentro(resultado == 1 ? "Entrou para a sua equipe." : "Equipe cheia: enviado ao armazenamento.",
                    JANELA_LARGURA / 2, 465, 18, COR_TEXTO);
        textoCentro("ENTER para continuar", JANELA_LARGURA / 2, 520, 18, COR_SUAVE);
    }
}

void interfaceEquipe(const EstadoJogo *g, int sel, int marcado, const char *msg)
{
    const Equipe *e = &g->equipe;
    int total = e->qtdAtivos + e->qtdArmazenamento;
    ClearBackground(COR_FUNDO);
    textoCentro("EQUIPE", JANELA_LARGURA / 2, 14, 30, COR_DESTAQUE);
    painel(10, 56, 380, 470);
    int inicio = sel > 14 ? sel - 14 : 0;
    for (int i = inicio; i < total && i < inicio + 15; i++) {
        int ativo = i < e->qtdAtivos;
        const Criatura *c = ativo ? &e->ativos[i] : &e->armazenamento[i - e->qtdAtivos];
        int y = 64 + (i - inicio) * 30;
        Color cor = i == sel ? COR_DESTAQUE : (criaturaEstaViva(c) ? COR_TEXTO : COR_SUAVE);
        if (i == sel) DrawText(">", 16, y, 16, cor);
        const char *tag = ativo ? (i == e->indiceAtivo ? "[ATIVA]" : "[Equipe]") : (i - e->qtdAtivos == marcado ? "[Marcada]" : "[Reserva]");
        DrawText(TextFormat("%s %s Nv%d", tag, c->nome, c->nivel), 34, y, 16, cor);
    }
    if (sel >= 0 && sel < total) {
        const Criatura *c = sel < e->qtdAtivos ? &e->ativos[sel] : &e->armazenamento[sel - e->qtdAtivos];
        painel(400, 56, 390, 470);
        desenharCriatura(595, 160, 1.2f, c, 1.0f);
        DrawText(c->nome, 420, 250, 22, COR_TEXTO);
        DrawText(TextFormat("%s | %s | Nv %d", nomeAfinidade(c->afinidade), nomeRaridade(c->raridade), c->nivel), 420, 280, 16, corAfinidade(c->afinidade));
        DrawText(TextFormat("HP %d/%d   Energia %d/%d", c->hp, c->hpMax, c->energia, c->energiaMax), 420, 310, 16, COR_TEXTO);
        DrawText(TextFormat("Ataque %d   Defesa %d   Velocidade %d", c->ataque, c->defesa, c->velocidade), 420, 336, 16, COR_TEXTO);
        DrawText(TextFormat("XP %d/%d", c->xp, c->nivel >= NIVEL_MAXIMO ? 0 : xpParaProximoNivel(c->nivel)), 420, 362, 16, COR_TEXTO);
        for (int i = 0; i < MAX_HABILIDADES; i++) {
            DrawText(TextFormat("%s (poder %d, EN %d)", c->habilidades[i].nome, c->habilidades[i].poder, c->habilidades[i].custoEnergia),
                     420, 400 + i * 26, 16, COR_SUAVE);
        }
    }
    if (msg != NULL && msg[0] != '\0') DrawText(msg, 16, 536, 16, COR_DESTAQUE);
    DrawText("ENTER: definir ativa / marcar reserva e trocar | ESC: voltar", 16, 566, 16, COR_SUAVE);
}

void interfacePausa(int sel, const char *msg)
{
    static const char *ops[] = {"Continuar", "Salvar jogo", "Ver a história", "Instruções", "Voltar ao menu"};
    DrawRectangle(0, 0, JANELA_LARGURA, JANELA_ALTURA, Fade(BLACK, 0.7f));
    painel(250, 150, 300, 330);
    textoCentro("PAUSA", JANELA_LARGURA / 2, 170, 34, COR_DESTAQUE);
    opcoes(ops, 5, sel, 290, 235, 22, 40, -1);
    if (msg != NULL && msg[0] != '\0') textoCentro(msg, JANELA_LARGURA / 2, 442, 16, COR_TEXTO);
}

void interfaceFim(int vitoria, const EstadoJogo *g, int sel, int temSave)
{
    static const char *opsV[] = {"Jogar novamente", "Voltar ao menu", "Sair"};
    static const char *opsD[] = {"Carregar último save", "Recomeçar", "Voltar ao menu", "Sair"};
    ClearBackground(vitoria ? (Color){16, 22, 30, 255} : (Color){26, 8, 12, 255});
    textoCentro(vitoria ? "VITÓRIA" : "DERROTA", JANELA_LARGURA / 2, 80, 64,
                vitoria ? COR_DESTAQUE : (Color){220, 70, 70, 255});
    textoCentro(vitoria ? "O Guardião Corrompido caiu. A corrupção recua." : "Suas criaturas não resistiram à corrupção...",
                JANELA_LARGURA / 2, 170, 22, COR_TEXTO);
    textoCentro(TextFormat("%s — %d pontos | %d criaturas conquistadas | %d vitórias",
                           g->jogador.nome, g->jogador.pontuacao, g->jogador.conquistadas, g->jogador.vitorias),
                JANELA_LARGURA / 2, 220, 18, COR_SUAVE);
    if (vitoria) opcoes(opsV, 3, sel, 300, 320, 26, 46, -1);
    else opcoes(opsD, 4, sel, 300, 320, 26, 46, temSave ? -1 : 0);
}

/* ---------- lore e encontro ---------- */

void interfaceLore(int indice, int total, const char *texto)
{
    ClearBackground(COR_FUNDO);
    for (int i = 0; i < 20; i++) {
        float t = (float)GetTime() * 0.5f + (float)i * 1.9f;
        DrawCircle(40 + (i * 97) % 740, 600 - (int)fmodf(t * 35.0f + (float)(i * 41), 620.0f),
                   2.0f + (float)(i % 3), Fade(COR_BORDA, 0.4f));
    }
    painel(50, 90, 700, 330);
    textoEmLinhas(texto, 80, 130, 22, 36, COR_TEXTO);
    DrawText(TextFormat("%d/%d", indice + 1, total), 690, 440, 18, COR_SUAVE);
    textoCentro("ENTER continua  |  ESC pula a história", JANELA_LARGURA / 2, 540, 18, COR_SUAVE);
}

/* Fases do encontro, em segundos */
#define ENC_ALERTA 0.6      /* "!" sobre o jogador e tela piscando */
#define ENC_FECHAR 0.4      /* faixas pretas fecham a tela */
#define ENC_FALA 1.6        /* só no chefe: fala do Guardião */
#define ENC_ABRIR 0.5       /* faixas abrem; inimigo desliza para dentro */

double interfaceEncontroDuracao(int chefe)
{
    return ENC_ALERTA + ENC_FECHAR + (chefe ? ENC_FALA : 0.0) + ENC_ABRIR;
}

static void faixasPretas(int altura)
{
    if (altura <= 0) return;
    DrawRectangle(0, 0, JANELA_LARGURA, altura, BLACK);
    DrawRectangle(0, JANELA_ALTURA - altura, JANELA_LARGURA, altura, BLACK);
}

void interfaceEncontro(const Mapa *m, const EstadoJogo *g, const Batalha *b, double tempo, int chefe)
{
    double fala = chefe ? ENC_FALA : 0.0;
    double inicioFala = ENC_ALERTA + ENC_FECHAR;
    double inicioAbrir = inicioFala + fala;

    if (tempo < ENC_ALERTA) {
        interfaceExploracao(m, g, "");
        int px = g->jogador.x * TAMANHO_TILE + 16;
        int py = g->jogador.y * TAMANHO_TILE;
        int salto = (int)(sinf((float)tempo * 18.0f) * 4.0f);
        DrawText("!", px - 5, py - 36 + salto, 40, COR_DESTAQUE);
        if (((int)(tempo * 16.0)) % 2 == 0) DrawRectangle(0, 0, JANELA_LARGURA, JANELA_ALTURA, Fade(WHITE, 0.25f));
    } else if (tempo < inicioFala) {
        interfaceExploracao(m, g, "");
        double p = (tempo - ENC_ALERTA) / ENC_FECHAR;
        faixasPretas((int)(p * JANELA_ALTURA / 2));
    } else if (chefe && tempo < inicioAbrir) {
        ClearBackground(BLACK);
        const Texture2D *spriteChefe = assetsGuardiaoBatalha();
        if (spriteChefe != NULL) {
            Rectangle origem = {0.0f, 0.0f, (float)spriteChefe->width, (float)spriteChefe->height};
            Rectangle destino = {20.0f, 130.0f, (float)(spriteChefe->width * 3), (float)(spriteChefe->height * 3)};
            Vector2 zero = {0.0f, 0.0f};
            DrawTexturePro(*spriteChefe, origem, destino, zero, 0.0f, WHITE);     /* ampliado 3x */
        } else {
            desenharInvocador(160, 280, 3.0f, COR_MANTO_CHEFE, COR_BRILHO_CHEFE, 1);
        }
        DrawText(NOME_CHEFE, 330, 200, 28, COR_BRILHO_CHEFE);
        textoEmLinhas(loreFalaChefe(), 330, 250, 22, 34, COR_TEXTO);
    } else {
        double p = (tempo - inicioAbrir) / ENC_ABRIR;
        if (p > 1.0) p = 1.0;
        double resta = 1.0 - p;
        desenharBatalha(b, g, BM_ACOES, 0, 0, (int)(resta * resta * 400.0));
        faixasPretas((int)(resta * JANELA_ALTURA / 2));

        /* faixa com o nome do inimigo, entrando pela esquerda */
        int faixaX = -(int)(resta * JANELA_LARGURA);
        DrawRectangle(faixaX, 288, JANELA_LARGURA, 44, Fade(BLACK, 0.75f));
        DrawRectangle(faixaX, 288, JANELA_LARGURA, 3, COR_BORDA);
        const char *nome = chefe ? b->treinador : TextFormat("%s  Nv %d", b->inimigo.nome, b->inimigo.nivel);
        textoCentro(nome, JANELA_LARGURA / 2 + faixaX, 297, 28, COR_DESTAQUE);
    }
}
