/* Cursed Souls — Ecos Amaldiçoados
 * Laço principal: máquina de estados que liga a lógica (batalha, regras, mapa,
 * ressurreição, arquivos) à interface Raylib. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "arquivos.h"
#include "assets.h"
#include "batalha.h"
#include "equipe.h"
#include "interface.h"
#include "lore.h"
#include "mapa.h"
#include "pontuacao.h"
#include "regras.h"
#include "ressurreicao.h"

#define ARQUIVO_RANKING "saves/ranking.txt"
#define ENCONTRO_PCT 15
#define TAM_NOME_TELA 20
#define PASSO_SEGUNDOS 0.13
#define ESPERA_RESULTADO 1.6

typedef enum {
    TELA_MENU, TELA_NOME, TELA_INICIAL, TELA_INSTRUCOES, TELA_RANKING,
    TELA_EXPLORACAO, TELA_BATALHA, TELA_RESSURREICAO, TELA_EQUIPE,
    TELA_PAUSA, TELA_VITORIA, TELA_DERROTA, TELA_ENCONTRO, TELA_LORE
} Tela;

typedef struct {
    Tela tela, telaAnterior;
    int sel, selAcao, selTroca, marcado;
    int sair, temSave, continuando;     /* continuando: a tela de nome serve para carregar um save */
    EstadoJogo jogo;
    Mapa *mapa;
    Batalha batalha;
    ModoBatalha modo;
    Criatura derrotada;
    int ressChance, ressResultado;
    double ressTempo;
    char nome[MAX_NOME];
    char aviso[96];
    char mensagem[96];
    double tempoMensagem;
    double cooldown;
    int pontuacaoRegistrada;
    double encTempo;                      /* tempo da animação de encontro */
    int encChefe;                         /* 1 = encontro com o chefe (animação mais longa) */
    int lorePainel;                       /* painel da história que está na tela */
    Tela loreVolta;                       /* tela para onde voltar ao terminar a história */
    RegistroPontuacao ranking[MAX_RANKING];
    int qtdRanking;
} App;

/* Mostra uma mensagem curta na tela de exploração por 4 segundos. */
static void mostrarMensagem(App *a, const char *texto)
{
    snprintf(a->mensagem, sizeof a->mensagem, "%s", texto);
    a->tempoMensagem = 4.0;
}

static int ir(App *a, Tela t)
{
    a->tela = t;
    a->sel = 0;
    return 1;
}

static int navegar(int *sel, int n)
{
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) *sel = (*sel + 1) % n;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) *sel = (*sel + n - 1) % n;
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
}

static void abrirLore(App *a, Tela volta)
{
    a->loreVolta = volta;
    a->lorePainel = 0;
    ir(a, TELA_LORE);
}

/* Abre a tela de nome: para um jogo novo (continuando = 0) ou para carregar um save (1). */
static void abrirNome(App *a, int continuando)
{
    a->nome[0] = '\0';
    a->aviso[0] = '\0';
    a->continuando = continuando;
    ir(a, TELA_NOME);
}

static int carregarRegiao(App *a, int regiao, int x, int y)
{
    Mapa *novo = mapaCriar(regiao);
    if (novo == NULL) {
        mostrarMensagem(a, "Esse caminho está bloqueado.");
        return 0;
    }
    mapaLiberar(a->mapa);
    a->mapa = novo;
    a->jogo.jogador.regiao = regiao;
    a->jogo.jogador.x = x;
    a->jogo.jogador.y = y;
    return 1;
}

static void iniciarNovoJogo(App *a, int especie)
{
    if (!estadoNovo(&a->jogo, a->nome, especie) || !carregarRegiao(a, 0, SPAWN_X, SPAWN_Y)) {
        snprintf(a->aviso, sizeof a->aviso, "Não foi possível iniciar o jogo.");
        ir(a, TELA_MENU);
        return;
    }
    a->pontuacaoRegistrada = 0;
    a->marcado = -1;
    char msg[96];
    snprintf(msg, sizeof msg, "Boa sorte, %s! Explore a grama escura para encontrar criaturas.", a->nome);
    mostrarMensagem(a, msg);
    abrirLore(a, TELA_EXPLORACAO);      /* história de abertura antes de jogar */
}

/* Carrega o save do jogador "nome" (arquivo saves/<nome>.sav). */
static int continuarJogo(App *a, const char *nome)
{
    char caminho[64];
    if (!arquivosCaminhoSave(nome, caminho, sizeof caminho)) {
        snprintf(a->aviso, sizeof a->aviso, "Nome inválido.");
        return 0;
    }
    ResultadoArquivo r = arquivosCarregar(caminho, &a->jogo);
    if (r != ARQ_OK) {
        snprintf(a->aviso, sizeof a->aviso, r == ARQ_ERRO_ABRIR ? "Nenhum jogo salvo para esse nome." : "Arquivo de save inválido ou corrompido.");
        return 0;
    }
    Jogador *j = &a->jogo.jogador;
    if (!carregarRegiao(a, j->regiao, j->x, j->y)) {
        snprintf(a->aviso, sizeof a->aviso, "Região do save inválida.");
        return 0;
    }
    if (!mapaCaminhavel(a->mapa, j->x, j->y)) {      /* posição salva inválida: volta ao início */
        j->x = SPAWN_X;
        j->y = SPAWN_Y;
    }
    equipeGarantirAtivoVivo(&a->jogo.equipe);
    a->pontuacaoRegistrada = 0;
    a->marcado = -1;
    char msg[96];
    snprintf(msg, sizeof msg, "Jogo carregado. Bem-vindo de volta, %s!", j->nome);
    mostrarMensagem(a, msg);
    a->aviso[0] = '\0';
    ir(a, TELA_EXPLORACAO);
    return 1;
}

/* Grava a partida no save do jogador atual. Retorna 1 se deu certo. */
static int salvarJogo(const App *a)
{
    char caminho[64];
    if (!arquivosCaminhoSave(a->jogo.jogador.nome, caminho, sizeof caminho)) return 0;
    return arquivosSalvar(caminho, &a->jogo) == ARQ_OK;
}

static int existeSaveDoJogador(const App *a)
{
    char caminho[64];
    return arquivosCaminhoSave(a->jogo.jogador.nome, caminho, sizeof caminho) && arquivosExiste(caminho);
}

static void carregarRanking(App *a)
{
    a->qtdRanking = pontuacaoCarregar(ARQUIVO_RANKING, a->ranking, MAX_RANKING);
}

static void finalizarPartida(App *a, int vitoria)
{
    if (!a->pontuacaoRegistrada) {
        pontuacaoRegistrar(ARQUIVO_RANKING, a->jogo.jogador.nome, a->jogo.jogador.pontuacao);
        a->pontuacaoRegistrada = 1;
    }
    a->temSave = existeSaveDoJogador(a);
    ir(a, vitoria ? TELA_VITORIA : TELA_DERROTA);
}

/* A batalha já está montada; antes dela, toca a animação de encontro. */
static void entrarBatalha(App *a, int chefe)
{
    a->modo = BM_ACOES;
    a->selAcao = 0;
    a->selTroca = 0;
    a->encTempo = 0.0;
    a->encChefe = chefe;
    ir(a, TELA_ENCONTRO);
}

static void iniciarBatalhaSelvagem(App *a, const Criatura *inimigo)
{
    equipeGarantirAtivoVivo(&a->jogo.equipe);
    batalhaIniciar(&a->batalha, inimigo);
    entrarBatalha(a, 0);
}

static void iniciarBatalhaChefe(App *a)
{
    Criatura time[MAX_EQUIPE];
    int n = chefeMontarEquipe(time, MAX_EQUIPE);
    if (n <= 0) {
        mostrarMensagem(a, "O Guardião não está aqui agora.");
        return;
    }
    equipeGarantirAtivoVivo(&a->jogo.equipe);
    batalhaIniciarTreinador(&a->batalha, NOME_CHEFE, time, n);
    entrarBatalha(a, 1);
}

/* ---------- exploração ---------- */

static void tentarMover(App *a, int dx, int dy)
{
    char msg[96];
    Jogador *j = &a->jogo.jogador;
    int nx = j->x + dx, ny = j->y + dy;

    /* o personagem vira para onde o jogador apertou, mesmo que a passagem esteja bloqueada */
    if (dx > 0) j->direcao = DIR_DIREITA;
    else if (dx < 0) j->direcao = DIR_ESQUERDA;
    else if (dy < 0) j->direcao = DIR_CIMA;
    else if (dy > 0) j->direcao = DIR_BAIXO;

    if (!mapaCaminhavel(a->mapa, nx, ny)) return;
    j->x = nx;
    j->y = ny;
    j->passo++;

    switch (mapaTile(a->mapa, nx, ny)) {
    case '>':
        if (carregarRegiao(a, j->regiao + 1, SPAWN_X, SPAWN_Y)) {
            snprintf(msg, sizeof msg, "Você entrou em: %s", a->mapa->nome);
            mostrarMensagem(a, msg);
        }
        break;
    case '<':
        if (carregarRegiao(a, j->regiao - 1, SPAWN_X_DIREITA, SPAWN_Y)) {
            snprintf(msg, sizeof msg, "Você voltou para: %s", a->mapa->nome);
            mostrarMensagem(a, msg);
        }
        break;
    case 'F':
        equipeCurarTodos(&a->jogo.equipe);
        if (j->pocoes < POCOES_INICIAIS) j->pocoes = POCOES_INICIAIS;
        mostrarMensagem(a, "A fogueira restaura suas criaturas e repõe as poções.");
        break;
    case 'B':
        if (!j->chefeDerrotado) {
            if (!equipeTemVivo(&a->jogo.equipe)) {
                mostrarMensagem(a, "Sua equipe está esgotada! Descanse numa fogueira (F).");
            } else {
                iniciarBatalhaChefe(a);
            }
        }
        break;
    case ',':
        if (rand() % 100 < ENCONTRO_PCT) {
            Criatura selvagem;
            if (!equipeTemVivo(&a->jogo.equipe)) {
                mostrarMensagem(a, "Sua equipe está esgotada! Descanse numa fogueira (F).");
            } else if (mapaGerarCriatura(a->mapa, &selvagem)) {
                iniciarBatalhaSelvagem(a, &selvagem);
            }
        }
        break;
    default:
        break;
    }
}

static void atualizarExploracao(App *a)
{
    a->cooldown -= GetFrameTime();
    if (a->cooldown <= 0.0) {
        int dx = 0, dy = 0;
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) dy = -1;
        else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) dy = 1;
        else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) dx = -1;
        else if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) dx = 1;
        if (dx != 0 || dy != 0) {
            tentarMover(a, dx, dy);
            a->cooldown = PASSO_SEGUNDOS;
        }
    }
    if (a->tela != TELA_EXPLORACAO) return;
    if (IsKeyPressed(KEY_E)) {
        a->marcado = -1;
        a->mensagem[0] = '\0';
        ir(a, TELA_EQUIPE);
    } else if (IsKeyPressed(KEY_ESCAPE)) {
        a->aviso[0] = '\0';
        ir(a, TELA_PAUSA);
    }
    if (a->tempoMensagem > 0.0) {
        a->tempoMensagem -= GetFrameTime();
        if (a->tempoMensagem <= 0.0) a->mensagem[0] = '\0';
    }
}

/* ---------- batalha ---------- */

static void concluirBatalha(App *a)
{
    Batalha *b = &a->batalha;
    switch (b->estado) {
    case BAT_VITORIA:
        if (b->ehChefe) {
            finalizarPartida(a, 1);
        } else {
            a->derrotada = b->inimigo;
            a->ressChance = ressurreicaoChance(b->inimigo.raridade, 0);
            a->ressResultado = -1;
            a->ressTempo = 0.0;
            a->aviso[0] = '\0';
            ir(a, TELA_RESSURREICAO);
        }
        break;
    case BAT_DERROTA:
        finalizarPartida(a, 0);
        break;
    default:
        mostrarMensagem(a, "Você escapou!");
        ir(a, TELA_EXPLORACAO);
        break;
    }
}

static void atualizarBatalha(App *a)
{
    Batalha *b = &a->batalha;
    EstadoJogo *g = &a->jogo;

    if (b->estado != BAT_EM_ANDAMENTO) {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) concluirBatalha(a);
        return;
    }
    if (b->precisaTrocar && a->modo != BM_TROCA_FORCADA) {
        a->modo = BM_TROCA_FORCADA;
        a->selTroca = 0;
    }

    if (a->modo == BM_ACOES) {
        if (navegar(&a->selAcao, 5)) {
            switch (a->selAcao) {
            case 0: batalhaExecutarTurno(b, g, ACAO_ATACAR, 0); break;
            case 1: batalhaExecutarTurno(b, g, ACAO_HABILIDADE, 0); break;
            case 2: a->modo = BM_TROCA; a->selTroca = 0; break;
            case 3: batalhaExecutarTurno(b, g, ACAO_ITEM, 0); break;
            default: batalhaExecutarTurno(b, g, ACAO_FUGIR, 0); break;
            }
        }
    } else {
        if (navegar(&a->selTroca, g->equipe.qtdAtivos)) {
            int ok = (a->modo == BM_TROCA_FORCADA)
                         ? batalhaTrocaForcada(b, g, a->selTroca)
                         : batalhaExecutarTurno(b, g, ACAO_TROCAR, a->selTroca);
            if (ok) a->modo = BM_ACOES;
        } else if (a->modo == BM_TROCA && IsKeyPressed(KEY_ESCAPE)) {
            a->modo = BM_ACOES;
        }
    }
    if (b->precisaTrocar && b->estado == BAT_EM_ANDAMENTO) a->modo = BM_TROCA_FORCADA;
}

/* ---------- ressurreição ---------- */

static void atualizarRessurreicao(App *a)
{
    if (a->ressResultado == -1) {
        int usar = 0, tentar = 0;
        if (IsKeyPressed(KEY_ENTER)) tentar = 1;
        if (IsKeyPressed(KEY_C)) {
            if (a->jogo.jogador.correntes > 0) { usar = 1; tentar = 1; }
            else snprintf(a->aviso, sizeof a->aviso, "Você não tem cargas de Correntes da Maldição.");
        }
        if (tentar) {
            ResultadoRess r = ressurreicaoTentar(&a->jogo, &a->derrotada, usar, ressurreicaoSortear());
            if (r == RESS_ERRO) {
                snprintf(a->aviso, sizeof a->aviso, "Não foi possível ressuscitar esta criatura.");
            } else {
                a->ressResultado = (r == RESS_FALHA) ? 0 : (r == RESS_SUCESSO_EQUIPE ? 1 : 2);
                a->ressTempo = 0.0;
                a->aviso[0] = '\0';
            }
        } else if (IsKeyPressed(KEY_ESCAPE)) {
            mostrarMensagem(a, "A essência se dissipou.");
            ir(a, TELA_EXPLORACAO);
        }
        return;
    }
    a->ressTempo += GetFrameTime();
    if (a->ressTempo > ESPERA_RESULTADO && IsKeyPressed(KEY_ENTER)) {
        if (a->ressResultado == 0) {
            mostrarMensagem(a, "A essência se dissipou.");
        } else {
            char msg[96];
            snprintf(msg, sizeof msg, "%s agora luta ao seu lado!", a->derrotada.nome);
            mostrarMensagem(a, msg);
        }
        ir(a, TELA_EXPLORACAO);
    }
}

/* ---------- equipe ---------- */

static void atualizarEquipe(App *a)
{
    Equipe *e = &a->jogo.equipe;
    int total = e->qtdAtivos + e->qtdArmazenamento;
    if (total <= 0) { ir(a, TELA_EXPLORACAO); return; }
    if (IsKeyPressed(KEY_ESCAPE)) { a->mensagem[0] = '\0'; ir(a, TELA_EXPLORACAO); return; }

    if (navegar(&a->sel, total)) {
        if (a->sel < e->qtdAtivos) {
            if (a->marcado >= 0) {
                equipeTrocarComArmazenamento(e, a->sel, a->marcado);
                equipeGarantirAtivoVivo(e);
                a->marcado = -1;
                snprintf(a->mensagem, sizeof a->mensagem, "Troca realizada.");
            } else if (equipeTrocarAtivo(e, a->sel)) {
                snprintf(a->mensagem, sizeof a->mensagem, "%s agora é a criatura ativa.", e->ativos[a->sel].nome);
            } else {
                snprintf(a->mensagem, sizeof a->mensagem, "Essa criatura já é a ativa ou está derrotada.");
            }
        } else {
            int idx = a->sel - e->qtdAtivos;
            a->marcado = (a->marcado == idx) ? -1 : idx;
            snprintf(a->mensagem, sizeof a->mensagem, a->marcado >= 0 ? "Escolha o slot da equipe que será trocado." : "Seleção cancelada.");
        }
    }
}

/* ---------- demais telas ---------- */

static void atualizarNome(App *a)
{
    size_t n = strlen(a->nome);
    int c;
    while ((c = GetCharPressed()) > 0) {
        if (c == ' ') c = '_';
        int ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-';
        if (ok && n < TAM_NOME_TELA) {
            a->nome[n++] = (char)c;
            a->nome[n] = '\0';
        }
    }
    if (IsKeyPressed(KEY_BACKSPACE) && n > 0) a->nome[n - 1] = '\0';
    if (IsKeyPressed(KEY_ESCAPE)) {
        a->aviso[0] = '\0';
        ir(a, TELA_MENU);
        return;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        if (!validarNome(a->nome)) {
            snprintf(a->aviso, sizeof a->aviso, "Digite um nome válido (letras, números, _ ou -).");
        } else if (a->continuando) {
            continuarJogo(a, a->nome);      /* se falhar, o aviso aparece nesta tela */
        } else {
            a->aviso[0] = '\0';
            ir(a, TELA_INICIAL);
        }
    }
}

static void atualizarPausa(App *a)
{
    if (IsKeyPressed(KEY_ESCAPE)) { ir(a, TELA_EXPLORACAO); return; }
    if (navegar(&a->sel, 5)) {
        switch (a->sel) {
        case 0: ir(a, TELA_EXPLORACAO); break;
        case 1: snprintf(a->aviso, sizeof a->aviso, salvarJogo(a) ? "Jogo salvo!" : "Erro ao salvar o jogo."); break;
        case 2: abrirLore(a, TELA_PAUSA); break;
        case 3: a->telaAnterior = TELA_PAUSA; ir(a, TELA_INSTRUCOES); break;
        default: a->aviso[0] = '\0'; ir(a, TELA_MENU); break;
        }
    }
}

static void atualizarFim(App *a, int vitoria)
{
    int n = vitoria ? 3 : 4;
    if (!navegar(&a->sel, n)) return;
    a->aviso[0] = '\0';
    if (vitoria) {
        if (a->sel == 0) abrirNome(a, 0);
        else if (a->sel == 1) ir(a, TELA_MENU);
        else a->sair = 1;
    } else {
        switch (a->sel) {
        case 0:
        {
            char nome[MAX_NOME];
            snprintf(nome, sizeof nome, "%s", a->jogo.jogador.nome);   /* cópia, pois o carregamento troca a->jogo */
            if (!continuarJogo(a, nome)) a->sel = 0;
            break;
        }
        case 1: abrirNome(a, 0); break;
        case 2: ir(a, TELA_MENU); break;
        default: a->sair = 1; break;
        }
    }
}

static void atualizarMenu(App *a)
{
    if (!navegar(&a->sel, 5)) return;
    a->aviso[0] = '\0';
    switch (a->sel) {
    case 0: abrirNome(a, 0); break;
    case 1: abrirNome(a, 1); break;
    case 2: a->telaAnterior = TELA_MENU; ir(a, TELA_INSTRUCOES); break;
    case 3: carregarRanking(a); ir(a, TELA_RANKING); break;
    default: a->sair = 1; break;
    }
}

static void atualizarEncontro(App *a)
{
    a->encTempo += GetFrameTime();
    int pular = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
    if (pular || a->encTempo >= interfaceEncontroDuracao(a->encChefe)) ir(a, TELA_BATALHA);
}

static void atualizarLore(App *a)
{
    if (IsKeyPressed(KEY_ESCAPE)) {
        ir(a, a->loreVolta);                    /* pula a história */
    } else if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (a->lorePainel + 1 < loreQtdPaineis()) a->lorePainel++;
        else ir(a, a->loreVolta);
    }
}

static void atualizar(App *a)
{
    switch (a->tela) {
    case TELA_MENU: atualizarMenu(a); break;
    case TELA_NOME: atualizarNome(a); break;
    case TELA_INICIAL: {
        int sel = a->sel;
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) sel = (sel + 1) % 3;
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) sel = (sel + 2) % 3;
        a->sel = sel;
        if (IsKeyPressed(KEY_ENTER)) iniciarNovoJogo(a, a->sel);
        else if (IsKeyPressed(KEY_ESCAPE)) ir(a, TELA_NOME);
        break;
    }
    case TELA_INSTRUCOES:
    case TELA_RANKING:
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
            Tela volta = (a->tela == TELA_INSTRUCOES) ? a->telaAnterior : TELA_MENU;
            ir(a, volta);
        }
        break;
    case TELA_EXPLORACAO: atualizarExploracao(a); break;
    case TELA_BATALHA: atualizarBatalha(a); break;
    case TELA_RESSURREICAO: atualizarRessurreicao(a); break;
    case TELA_EQUIPE: atualizarEquipe(a); break;
    case TELA_PAUSA: atualizarPausa(a); break;
    case TELA_ENCONTRO: atualizarEncontro(a); break;
    case TELA_LORE: atualizarLore(a); break;
    case TELA_VITORIA: atualizarFim(a, 1); break;
    case TELA_DERROTA: atualizarFim(a, 0); break;
    }
}

static void desenhar(const App *a)
{
    switch (a->tela) {
    case TELA_MENU: interfaceMenu(a->sel, a->aviso); break;
    case TELA_NOME:
        interfaceNome(a->continuando ? "Qual jogo você quer continuar?" : "Qual é o seu nome, invocador?", a->nome, a->aviso);
        break;
    case TELA_INICIAL: interfaceInicial(a->sel); break;
    case TELA_INSTRUCOES: interfaceInstrucoes(); break;
    case TELA_RANKING: interfaceRanking(a->ranking, a->qtdRanking); break;
    case TELA_EXPLORACAO: interfaceExploracao(a->mapa, &a->jogo, a->mensagem); break;
    case TELA_BATALHA: interfaceBatalha(&a->batalha, &a->jogo, a->modo, a->selAcao, a->selTroca); break;
    case TELA_RESSURREICAO:
        interfaceRessurreicao(&a->derrotada, a->ressChance, a->jogo.jogador.correntes,
                              a->ressResultado, a->ressTempo, a->aviso);
        break;
    case TELA_EQUIPE: interfaceEquipe(&a->jogo, a->sel, a->marcado, a->mensagem); break;
    case TELA_PAUSA:
        interfaceExploracao(a->mapa, &a->jogo, "");
        interfacePausa(a->sel, a->aviso);
        break;
    case TELA_ENCONTRO:
        interfaceEncontro(a->mapa, &a->jogo, &a->batalha, a->encTempo, a->encChefe);
        break;
    case TELA_LORE: interfaceLore(a->lorePainel, loreQtdPaineis(), lorePainel(a->lorePainel)); break;
    case TELA_VITORIA: interfaceFim(1, &a->jogo, a->sel, a->temSave); break;
    case TELA_DERROTA: interfaceFim(0, &a->jogo, a->sel, a->temSave); break;
    }
}

int main(void)
{
    App app;
    memset(&app, 0, sizeof app);
    srand((unsigned)time(NULL));
    estadoInicializar(&app.jogo);
    app.tela = TELA_MENU;
    app.marcado = -1;

    InitWindow(JANELA_LARGURA, JANELA_ALTURA, "Cursed Souls — Ecos Amaldiçoados");
    SetExitKey(KEY_NULL);                 /* ESC é usado pelo jogo, não fecha a janela */
    SetTargetFPS(60);
    assetsCarregar();                     /* sprites; se faltar algum, o jogo usa formas simples */

    while (!WindowShouldClose() && !app.sair) {
        atualizar(&app);
        BeginDrawing();
        desenhar(&app);
        EndDrawing();
    }

    assetsLiberar();
    CloseWindow();
    mapaLiberar(app.mapa);
    estadoLiberar(&app.jogo);
    return 0;
}
