#include "regras.h"
#include "mapa.h"
#include <stdio.h>
#include <string.h>

void estadoInicializar(EstadoJogo *g)
{
    if (g == NULL) return;
    memset(g, 0, sizeof *g);
    equipeInicializar(&g->equipe);
}

void estadoLiberar(EstadoJogo *g)
{
    if (g == NULL) return;
    equipeLiberar(&g->equipe);
}

int estadoNovo(EstadoJogo *g, const char *nome, int especieInicial)
{
    Criatura inicial;
    if (g == NULL || !validarNome(nome)) return 0;
    if (especieInicial < 0 || especieInicial >= TOTAL_ESPECIES) return 0;
    if (!criaturaCriar(&inicial, especieInicial, 5)) return 0;

    estadoLiberar(g);                    /* g precisa ter sido inicializado */
    estadoInicializar(g);
    snprintf(g->jogador.nome, sizeof g->jogador.nome, "%s", nome);
    g->jogador.regiao = 0;
    g->jogador.x = SPAWN_X;
    g->jogador.y = SPAWN_Y;
    g->jogador.correntes = CORRENTES_INICIAIS;
    g->jogador.pocoes = POCOES_INICIAIS;
    return equipeAdicionar(&g->equipe, &inicial) != 0;
}

int limitar(int valor, int minimo, int maximo)
{
    if (valor < minimo) return minimo;
    if (valor > maximo) return maximo;
    return valor;
}

int calcularDano(const Criatura *atq, const Criatura *def, const Habilidade *h, int variacaoPct)
{
    if (atq == NULL || def == NULL || h == NULL) return 0;
    long base = (long)h->poder * (10 + atq->ataque) / (10 + criaturaDefesaEfetiva(def));
    base = base * multiplicadorAfinidade(atq->afinidade, def->afinidade) / 100;
    base = base * limitar(variacaoPct, 50, 150) / 100;
    return base < 1 ? 1 : (int)base;
}

int xpParaProximoNivel(int nivel)
{
    return 15 + 10 * nivel;
}

int ganharExperiencia(Criatura *c, int xp)
{
    int ganhos = 0;
    if (c == NULL || xp <= 0 || c->nivel >= NIVEL_MAXIMO) return 0;
    c->xp += xp;
    while (c->nivel < NIVEL_MAXIMO && c->xp >= xpParaProximoNivel(c->nivel)) {
        c->xp -= xpParaProximoNivel(c->nivel);
        criaturaSubirNivel(c);
        ganhos++;
    }
    if (c->nivel >= NIVEL_MAXIMO) c->xp = 0;
    return ganhos;
}

int xpPorDerrota(const Criatura *i)
{
    return 12 + i->nivel * 6 + (int)i->raridade * 10;
}

int pontosPorDerrota(const Criatura *i)
{
    return i->nivel * 10 + (int)i->raridade * 25;
}

int pontosPorConquista(const Criatura *c)
{
    return 50 + 25 * (int)c->raridade;
}

int validarNome(const char *nome)
{
    if (nome == NULL) return 0;
    size_t n = strlen(nome);
    if (n == 0 || n >= MAX_NOME) return 0;
    for (size_t i = 0; i < n; i++) {
        char c = nome[i];
        int ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                 (c >= '0' && c <= '9') || c == '_' || c == '-';
        if (!ok) return 0;
    }
    return 1;
}
