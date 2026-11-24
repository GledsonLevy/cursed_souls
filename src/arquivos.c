#include "arquivos.h"
#include "mapa.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define CRIAR_PASTA(p) _mkdir(p)
#else
#define CRIAR_PASTA(p) mkdir((p), 0755)
#endif

#define CABECALHO "CURSEDSOULS"
#define VERSAO_SAVE 1

int arquivosExiste(const char *caminho)
{
    if (caminho == NULL) return 0;
    FILE *f = fopen(caminho, "r");
    if (f == NULL) return 0;
    fclose(f);
    return 1;
}

int arquivosCaminhoSave(const char *nome, char *saida, size_t tam)
{
    if (saida == NULL || tam == 0) return 0;
    saida[0] = '\0';
    if (!validarNome(nome)) return 0;            /* só letras, números, _ e - */
    int n = snprintf(saida, tam, "saves/%s.sav", nome);
    if (n < 0 || (size_t)n >= tam) {
        saida[0] = '\0';
        return 0;
    }
    return 1;
}

/* Cria a pasta onde o arquivo ficará (ex.: "saves" para "saves/jogo.sav"). */
int arquivosGarantirDiretorio(const char *caminho)
{
    char pasta[256];
    if (caminho == NULL || strlen(caminho) >= sizeof pasta) return 0;
    snprintf(pasta, sizeof pasta, "%s", caminho);
    char *barra = strrchr(pasta, '/');
    if (barra == NULL) return 1;         /* arquivo no diretório atual */
    *barra = '\0';
    struct stat st;
    if (stat(pasta, &st) == 0) return S_ISDIR(st.st_mode);
    return CRIAR_PASTA(pasta) == 0;
}

static void escreverCriatura(FILE *f, const Criatura *c)
{
    fprintf(f, "%d %d %d %d %d %d %d %d %d %d\n", c->especieId, c->nivel, c->xp,
            c->hp, c->hpMax, c->ataque, c->defesa, c->velocidade, c->energia, c->energiaMax);
}

ResultadoArquivo arquivosSalvar(const char *caminho, const EstadoJogo *g)
{
    if (caminho == NULL || g == NULL || !validarNome(g->jogador.nome)) return ARQ_ERRO_DADOS;
    if (!arquivosGarantirDiretorio(caminho)) return ARQ_ERRO_ABRIR;

    FILE *f = fopen(caminho, "w");
    if (f == NULL) return ARQ_ERRO_ABRIR;

    const Jogador *j = &g->jogador;
    const Equipe *e = &g->equipe;
    fprintf(f, "%s %d\n", CABECALHO, VERSAO_SAVE);
    fprintf(f, "%s %d %d %d %d %d %d %d %d %d\n", j->nome, j->regiao, j->x, j->y,
            j->pontuacao, j->correntes, j->pocoes, j->vitorias, j->conquistadas,
            j->chefeDerrotado);
    fprintf(f, "%d %d %d\n", e->qtdAtivos, e->indiceAtivo, e->qtdArmazenamento);
    for (int i = 0; i < e->qtdAtivos; i++) escreverCriatura(f, &e->ativos[i]);
    for (int i = 0; i < e->qtdArmazenamento; i++) escreverCriatura(f, &e->armazenamento[i]);

    if (fclose(f) != 0) return ARQ_ERRO_ABRIR;
    return ARQ_OK;
}

/* Lê uma criatura e confere se os valores são possíveis. Retorna 1 se ok. */
static int lerCriatura(FILE *f, Criatura *c)
{
    int id, nivel, xp, hp, hpMax, atk, def, vel, en, enMax;
    if (fscanf(f, "%d %d %d %d %d %d %d %d %d %d", &id, &nivel, &xp, &hp, &hpMax,
               &atk, &def, &vel, &en, &enMax) != 10) {
        return 0;
    }
    if (xp < 0 || hpMax < 1 || hpMax > 9999 || hp < 0 || hp > hpMax) return 0;
    if (atk < 0 || atk > 999 || def < 0 || def > 999 || vel < 0 || vel > 999) return 0;
    if (enMax < 0 || enMax > 99 || en < 0 || en > enMax) return 0;
    if (!criaturaCriar(c, id, nivel)) return 0;
    c->xp = xp;
    c->hp = hp;
    c->hpMax = hpMax;
    c->ataque = atk;
    c->defesa = def;
    c->velocidade = vel;
    c->energia = en;
    c->energiaMax = enMax;
    return 1;
}

static int jogadorValido(const Jogador *j)
{
    if (!validarNome(j->nome)) return 0;
    if (j->regiao < 0 || j->regiao >= TOTAL_REGIOES) return 0;
    if (j->x < 0 || j->x >= MAPA_LARGURA || j->y < 0 || j->y >= MAPA_ALTURA) return 0;
    if (j->pontuacao < 0 || j->vitorias < 0 || j->conquistadas < 0) return 0;
    if (j->correntes < 0 || j->correntes > 99 || j->pocoes < 0 || j->pocoes > 99) return 0;
    return j->chefeDerrotado == 0 || j->chefeDerrotado == 1;
}

/* Lê o arquivo inteiro para "novo". Quem chama libera "novo" se houver erro. */
static ResultadoArquivo lerEstado(FILE *f, EstadoJogo *novo)
{
    char cab[16];
    int versao, qtdAtivos, indiceAtivo, qtdArmaz;
    Jogador *j = &novo->jogador;

    if (fscanf(f, "%15s %d", cab, &versao) != 2) return ARQ_ERRO_FORMATO;
    if (strcmp(cab, CABECALHO) != 0 || versao != VERSAO_SAVE) return ARQ_ERRO_FORMATO;
    if (fscanf(f, "%31s %d %d %d %d %d %d %d %d %d", j->nome, &j->regiao, &j->x, &j->y,
               &j->pontuacao, &j->correntes, &j->pocoes, &j->vitorias, &j->conquistadas,
               &j->chefeDerrotado) != 10) {
        return ARQ_ERRO_FORMATO;
    }
    if (fscanf(f, "%d %d %d", &qtdAtivos, &indiceAtivo, &qtdArmaz) != 3) return ARQ_ERRO_FORMATO;

    if (!jogadorValido(j)) return ARQ_ERRO_DADOS;
    if (qtdAtivos < 1 || qtdAtivos > MAX_EQUIPE) return ARQ_ERRO_DADOS;
    if (indiceAtivo < 0 || indiceAtivo >= qtdAtivos) return ARQ_ERRO_DADOS;
    if (qtdArmaz < 0 || qtdArmaz > MAX_ARMAZENAMENTO) return ARQ_ERRO_DADOS;

    for (int i = 0; i < qtdAtivos + qtdArmaz; i++) {
        Criatura c;
        if (!lerCriatura(f, &c)) return ARQ_ERRO_FORMATO;
        if (equipeAdicionar(&novo->equipe, &c) == 0) return ARQ_ERRO_FORMATO;
    }
    novo->equipe.indiceAtivo = indiceAtivo;
    return ARQ_OK;
}

ResultadoArquivo arquivosCarregar(const char *caminho, EstadoJogo *g)
{
    if (caminho == NULL || g == NULL) return ARQ_ERRO_DADOS;
    FILE *f = fopen(caminho, "r");
    if (f == NULL) return ARQ_ERRO_ABRIR;

    EstadoJogo novo;
    estadoInicializar(&novo);
    ResultadoArquivo res = lerEstado(f, &novo);
    fclose(f);

    if (res != ARQ_OK) {                 /* erro: descarta o que foi lido e mantém o jogo atual */
        estadoLiberar(&novo);
        return res;
    }
    estadoLiberar(g);
    *g = novo;
    return ARQ_OK;
}
