#include "batalha.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define POCAO_CURA_PCT 40
#define MOD_DEFESA_MIN (-8)

/* Escreve uma mensagem no log da batalha. Funciona como o printf. */
static void logMsg(Batalha *b, const char *fmt, ...)
{
    char buf[BATALHA_LINHA_TAM];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    if (b->qtdLog == BATALHA_LINHAS) {
        memmove(b->log[0], b->log[1], (BATALHA_LINHAS - 1) * BATALHA_LINHA_TAM);
        b->qtdLog--;
    }
    snprintf(b->log[b->qtdLog++], BATALHA_LINHA_TAM, "%s", buf);
}

void batalhaIniciar(Batalha *b, const Criatura *inimigo)
{
    if (b == NULL || inimigo == NULL) return;
    memset(b, 0, sizeof *b);
    b->inimigo = *inimigo;
    b->inimigo.modDefesa = 0;
    b->estado = BAT_EM_ANDAMENTO;
    logMsg(b, "Um %s selvagem (Nv %d) apareceu!", inimigo->nome, inimigo->nivel);
}

void batalhaIniciarTreinador(Batalha *b, const char *nome, const Criatura *time, int qtd)
{
    if (b == NULL || nome == NULL || time == NULL || qtd <= 0 || qtd > MAX_EQUIPE) return;
    memset(b, 0, sizeof *b);
    snprintf(b->treinador, sizeof b->treinador, "%s", nome);
    for (int i = 0; i < qtd; i++) b->timeInimigo[i] = time[i];
    b->qtdTimeInimigo = qtd;
    b->indiceInimigo = 0;
    b->inimigo = b->timeInimigo[0];
    b->inimigo.modDefesa = 0;
    b->ehChefe = 1;
    b->estado = BAT_EM_ANDAMENTO;
    logMsg(b, "%s deseja batalhar!", b->treinador);
    logMsg(b, "%s envia %s!", b->treinador, b->inimigo.nome);
}

static const Habilidade *habilidadeInimigo(const Criatura *i)
{
    const Habilidade *esp = &i->habilidades[1];
    if (esp->custoEnergia <= i->energia && rand() % 100 < 40) return esp;
    return &i->habilidades[0];
}

static void aplicarHabilidade(Batalha *b, Criatura *atq, Criatura *def, const Habilidade *h)
{
    atq->energia -= h->custoEnergia;
    if (atq->energia < 0) atq->energia = 0;

    int mult = multiplicadorAfinidade(atq->afinidade, def->afinidade);
    int dano = calcularDano(atq, def, h, 90 + rand() % 21);
    def->hp -= dano;
    if (def->hp < 0) def->hp = 0;
    logMsg(b, "%s usou %s: %d de dano.", atq->nome, h->nome, dano);
    if (mult > 100) logMsg(b, "Foi super efetivo!");
    else if (mult < 100) logMsg(b, "Não foi muito efetivo...");

    if (h->efeito == EFEITO_REDUZ_DEFESA && def->hp > 0) {
        def->modDefesa -= 2;
        if (def->modDefesa < MOD_DEFESA_MIN) def->modDefesa = MOD_DEFESA_MIN;
        logMsg(b, "A defesa de %s caiu!", def->nome);
    } else if (h->efeito == EFEITO_DRENAR) {
        int cura = dano / 2 > 0 ? dano / 2 : 1;
        atq->hp += cura;
        if (atq->hp > atq->hpMax) atq->hp = atq->hpMax;
        logMsg(b, "%s drenou %d de vida.", atq->nome, cura);
    }
}

/* XP e pontos pela criatura inimiga que acabou de cair (vale para cada uma do chefe). */
static void concederRecompensas(Batalha *b, EstadoJogo *g)
{
    Equipe *e = &g->equipe;
    int xp = xpPorDerrota(&b->inimigo);
    int pontos = pontosPorDerrota(&b->inimigo);

    b->xpGanho += xp;
    b->pontosGanhos += pontos;
    g->jogador.pontuacao += pontos;

    for (int i = 0; i < e->qtdAtivos; i++) {
        Criatura *c = &e->ativos[i];
        if (!criaturaEstaViva(c)) continue;
        int parcela = (i == e->indiceAtivo) ? xp : xp / 2;
        int sobe = ganharExperiencia(c, parcela);
        if (i == e->indiceAtivo) logMsg(b, "%s ganhou %d XP.", c->nome, parcela);
        if (sobe > 0) logMsg(b, "%s subiu para o nível %d!", c->nome, c->nivel);
    }
}

static void aoDerrotarInimigo(Batalha *b, EstadoJogo *g)
{
    logMsg(b, "%s foi derrotado!", b->inimigo.nome);
    concederRecompensas(b, g);

    /* o invocador ainda tem criaturas: envia a próxima e a batalha continua */
    if (b->indiceInimigo + 1 < b->qtdTimeInimigo) {
        b->indiceInimigo++;
        b->inimigo = b->timeInimigo[b->indiceInimigo];
        b->inimigo.modDefesa = 0;
        logMsg(b, "%s envia %s!", b->treinador, b->inimigo.nome);
        return;
    }

    b->estado = BAT_VITORIA;
    g->jogador.vitorias++;
    if (b->ehChefe) {
        g->jogador.chefeDerrotado = 1;
        g->jogador.pontuacao += BONUS_PONTOS_CHEFE;
        b->pontosGanhos += BONUS_PONTOS_CHEFE;
        logMsg(b, "%s foi vencido!", b->treinador);
    }
    if (g->jogador.vitorias % 5 == 0 && g->jogador.correntes < 99) {
        g->jogador.correntes++;
        logMsg(b, "As Correntes da Maldição ficaram mais fortes (+1).");
    }
    equipeLimparModificadores(&g->equipe);
}

static void aoCairAtiva(Batalha *b, EstadoJogo *g, const Criatura *ativa)
{
    logMsg(b, "%s caiu em batalha!", ativa->nome);
    if (equipeTemVivo(&g->equipe)) {
        b->precisaTrocar = 1;
    } else {
        b->estado = BAT_DERROTA;
        logMsg(b, "Todas as suas criaturas caíram...");
        equipeLimparModificadores(&g->equipe);
    }
}

static int jogadorAtaca(Batalha *b, Criatura *ativa, const Habilidade *h)
{
    aplicarHabilidade(b, ativa, &b->inimigo, h);
    return b->inimigo.hp <= 0;
}

static int inimigoAtaca(Batalha *b, Criatura *ativa)
{
    aplicarHabilidade(b, &b->inimigo, ativa, habilidadeInimigo(&b->inimigo));
    return ativa->hp <= 0;
}

static void fimDeTurno(Batalha *b, Criatura *ativa)
{
    if (b->estado != BAT_EM_ANDAMENTO || b->precisaTrocar) return;
    if (ativa->energia < ativa->energiaMax) ativa->energia++;
    if (b->inimigo.energia < b->inimigo.energiaMax) b->inimigo.energia++;
}

int batalhaExecutarTurno(Batalha *b, EstadoJogo *g, AcaoBatalha acao, int parametro)
{
    if (b == NULL || g == NULL) return 0;
    if (b->estado != BAT_EM_ANDAMENTO || b->precisaTrocar) return 0;
    Criatura *ativa = equipeAtiva(&g->equipe);
    if (ativa == NULL || !criaturaEstaViva(ativa)) return 0;

    const Habilidade *h = NULL;
    switch (acao) {
    case ACAO_ATACAR:
        h = &ativa->habilidades[0];
        break;
    case ACAO_HABILIDADE:
        h = &ativa->habilidades[1];
        if (h->custoEnergia > ativa->energia) {
            logMsg(b, "Energia insuficiente para %s!", h->nome);
            return 0;
        }
        break;
    case ACAO_TROCAR:
        if (parametro < 0 || parametro >= g->equipe.qtdAtivos ||
            parametro == g->equipe.indiceAtivo ||
            !criaturaEstaViva(&g->equipe.ativos[parametro])) {
            logMsg(b, "Não é possível trocar para essa criatura.");
            return 0;
        }
        break;
    case ACAO_ITEM:
        if (g->jogador.pocoes <= 0) { logMsg(b, "Você não tem poções."); return 0; }
        if (ativa->hp >= ativa->hpMax) { logMsg(b, "A vida já está cheia."); return 0; }
        break;
    case ACAO_FUGIR:
        if (b->ehChefe) { logMsg(b, "Não é possível fugir deste combate!"); return 0; }
        break;
    default:
        return 0;
    }

    b->turno++;

    if (acao == ACAO_ATACAR || acao == ACAO_HABILIDADE) {
        int jogadorPrimeiro = ativa->velocidade >= b->inimigo.velocidade;
        if (jogadorPrimeiro) {
            if (jogadorAtaca(b, ativa, h)) { aoDerrotarInimigo(b, g); fimDeTurno(b, ativa); return 1; }
            if (inimigoAtaca(b, ativa)) { aoCairAtiva(b, g, ativa); return 1; }
        } else {
            if (inimigoAtaca(b, ativa)) { aoCairAtiva(b, g, ativa); return 1; }
            if (jogadorAtaca(b, ativa, h)) { aoDerrotarInimigo(b, g); fimDeTurno(b, ativa); return 1; }
        }
        fimDeTurno(b, ativa);
        return 1;
    }

    if (acao == ACAO_TROCAR) {
        ativa->modDefesa = 0;
        equipeTrocarAtivo(&g->equipe, parametro);
        ativa = equipeAtiva(&g->equipe);
        logMsg(b, "Vai, %s!", ativa->nome);
    } else if (acao == ACAO_ITEM) {
        int cura = ativa->hpMax * POCAO_CURA_PCT / 100;
        if (cura < 10) cura = 10;
        ativa->hp += cura;
        if (ativa->hp > ativa->hpMax) ativa->hp = ativa->hpMax;
        g->jogador.pocoes--;
        logMsg(b, "%s recuperou vida com a poção.", ativa->nome);
    } else {   /* ACAO_FUGIR */
        int chance = limitar(50 + (ativa->velocidade - b->inimigo.velocidade) * 5, 10, 90);
        if (rand() % 100 < chance) {
            b->estado = BAT_FUGA;
            logMsg(b, "Você escapou com segurança!");
            equipeLimparModificadores(&g->equipe);
            return 1;
        }
        logMsg(b, "Não conseguiu fugir!");
    }

    if (inimigoAtaca(b, ativa)) { aoCairAtiva(b, g, ativa); return 1; }
    fimDeTurno(b, ativa);
    return 1;
}

int batalhaTrocaForcada(Batalha *b, EstadoJogo *g, int indice)
{
    if (b == NULL || g == NULL || !b->precisaTrocar) return 0;
    if (!equipeTrocarAtivo(&g->equipe, indice)) return 0;
    b->precisaTrocar = 0;
    logMsg(b, "Vai, %s!", g->equipe.ativos[indice].nome);
    return 1;
}
