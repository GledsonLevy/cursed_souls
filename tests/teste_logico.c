/* Testes da lógica do jogo (sem Raylib). Executar com: make teste */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arquivos.h"
#include "lore.h"
#include "batalha.h"
#include "mapa.h"
#include "pontuacao.h"
#include "ressurreicao.h"
#include "regras.h"

static int total = 0, falhas = 0;

#define VERIFICAR(cond, msg) do { \
    total++; \
    if (!(cond)) { falhas++; printf("    [FALHA] %s (linha %d)\n", msg, __LINE__); } \
} while (0)

static EstadoJogo novoJogo(void)
{
    EstadoJogo g;
    estadoInicializar(&g);
    estadoNovo(&g, "Teste", 0);
    return g;
}

static void ct01_dano_e_afinidade(void)
{
    printf("CT-01 dano mínimo e vantagem de afinidade\n");
    Criatura fogo, gelo, sombria;
    criaturaCriar(&fogo, 1, 5);
    criaturaCriar(&gelo, 2, 5);
    criaturaCriar(&sombria, 0, 5);
    Habilidade fraca = {"Teste", 1, 0, EFEITO_NENHUM};
    gelo.defesa = 900;
    VERIFICAR(calcularDano(&fogo, &gelo, &fraca, 100) >= 1, "dano nunca menor que 1");
    gelo.defesa = 5;
    VERIFICAR(multiplicadorAfinidade(AFIN_FOGO, AFIN_GELO) == 150, "Fogo > Gelo");
    VERIFICAR(multiplicadorAfinidade(AFIN_GELO, AFIN_FOGO) == 67, "Gelo < Fogo");
    VERIFICAR(multiplicadorAfinidade(AFIN_SOMBRIA, AFIN_SOMBRIA) == 100, "mesma afinidade neutra");
    Habilidade forte = {"Forte", 10, 0, EFEITO_NENHUM};
    VERIFICAR(calcularDano(&fogo, &gelo, &forte, 100) > calcularDano(&fogo, &sombria, &forte, 100),
              "vantagem causa mais dano que neutro");
}

static void ct02_chances_ressurreicao(void)
{
    printf("CT-02 chances de ressurreição por raridade\n");
    VERIFICAR(ressurreicaoChance(RAR_COMUM, 0) == 75, "comum 75%");
    VERIFICAR(ressurreicaoChance(RAR_INCOMUM, 0) == 55, "incomum 55%");
    VERIFICAR(ressurreicaoChance(RAR_RARA, 0) == 35, "rara 35%");
    VERIFICAR(ressurreicaoChance(RAR_EPICA, 0) == 20, "épica 20%");
    VERIFICAR(ressurreicaoChance(RAR_LENDARIA, 0) == 10, "lendária 10%");
    VERIFICAR(ressurreicaoChance(RAR_COMUM, 1) == 95, "limite máximo 95%");
    VERIFICAR(ressurreicaoChance((Raridade)99, 0) == 1, "raridade inválida não quebra");
}

static void ct03_tentativa_ressurreicao(void)
{
    printf("CT-03 tentativa de ressurreição (sucesso, falha, correntes)\n");
    EstadoJogo g = novoJogo();
    Criatura raposa;
    criaturaCriar(&raposa, 1, 3);
    raposa.hp = 0;
    VERIFICAR(ressurreicaoTentar(&g, &raposa, 0, 74) == RESS_SUCESSO_EQUIPE, "sorteio 74 < 75 vence");
    VERIFICAR(g.equipe.qtdAtivos == 2, "criatura entrou na equipe");
    VERIFICAR(g.equipe.ativos[1].hp == g.equipe.ativos[1].hpMax / 2, "volta com metade da vida");
    VERIFICAR(ressurreicaoTentar(&g, &raposa, 0, 75) == RESS_FALHA, "sorteio 75 falha");
    VERIFICAR(g.equipe.qtdAtivos == 2, "falha não adiciona criatura");
    int antes = g.jogador.correntes;
    VERIFICAR(ressurreicaoTentar(&g, &raposa, 1, 90) == RESS_SUCESSO_EQUIPE, "correntes: 90 < 95");
    VERIFICAR(g.jogador.correntes == antes - 1, "correntes consumidas");
    g.jogador.correntes = 0;
    VERIFICAR(ressurreicaoTentar(&g, &raposa, 1, 0) == RESS_ERRO, "sem cargas = erro");
    VERIFICAR(ressurreicaoTentar(&g, &raposa, 0, 100) == RESS_ERRO, "sorteio fora do intervalo");
    VERIFICAR(ressurreicaoTentar(&g, NULL, 0, 0) == RESS_ERRO, "criatura nula");
    estadoLiberar(&g);
}

static void ct04_limite_equipe(void)
{
    printf("CT-04 equipe limitada a 3 e armazenamento dinâmico\n");
    EstadoJogo g = novoJogo();
    Criatura c;
    criaturaCriar(&c, 1, 2);
    VERIFICAR(equipeAdicionar(&g.equipe, &c) == 1, "2ª entra na equipe");
    VERIFICAR(equipeAdicionar(&g.equipe, &c) == 1, "3ª entra na equipe");
    VERIFICAR(equipeAdicionar(&g.equipe, &c) == 2, "4ª vai ao armazenamento");
    for (int i = 0; i < 20; i++) equipeAdicionar(&g.equipe, &c);
    VERIFICAR(g.equipe.qtdArmazenamento == 21, "armazenamento cresce (realloc)");
    VERIFICAR(g.equipe.qtdAtivos == MAX_EQUIPE, "equipe nunca passa de 3");
    VERIFICAR(equipeTrocarComArmazenamento(&g.equipe, 0, 999) == 0, "índice inválido rejeitado");
    VERIFICAR(equipeTrocarComArmazenamento(&g.equipe, 0, 0) == 1, "troca válida");
    estadoLiberar(&g);
}

static void ct05_troca_invalida(void)
{
    printf("CT-05 troca de criatura ativa inválida\n");
    EstadoJogo g = novoJogo();
    Criatura c;
    criaturaCriar(&c, 2, 4);
    equipeAdicionar(&g.equipe, &c);
    c.hp = 0;
    equipeAdicionar(&g.equipe, &c);
    VERIFICAR(equipeTrocarAtivo(&g.equipe, -1) == 0, "índice negativo");
    VERIFICAR(equipeTrocarAtivo(&g.equipe, 7) == 0, "índice acima do limite");
    VERIFICAR(equipeTrocarAtivo(&g.equipe, 0) == 0, "já é a ativa");
    VERIFICAR(equipeTrocarAtivo(&g.equipe, 2) == 0, "criatura derrotada");
    VERIFICAR(equipeTrocarAtivo(&g.equipe, 1) == 1, "troca válida");
    estadoLiberar(&g);
}

static void ct06_salvar_carregar(void)
{
    printf("CT-06 salvar e carregar o estado completo\n");
    const char *arq = "saves/teste.sav";
    EstadoJogo g = novoJogo();
    Criatura c;
    criaturaCriar(&c, 4, 8);
    c.hp = 7;
    for (int i = 0; i < 5; i++) equipeAdicionar(&g.equipe, &c);
    g.jogador.pontuacao = 1234;
    g.jogador.regiao = 1;
    g.jogador.x = 5;
    g.jogador.y = 9;
    g.equipe.indiceAtivo = 1;
    VERIFICAR(arquivosSalvar(arq, &g) == ARQ_OK, "salvar retorna OK");

    EstadoJogo l;
    estadoInicializar(&l);
    VERIFICAR(arquivosCarregar(arq, &l) == ARQ_OK, "carregar retorna OK");
    VERIFICAR(strcmp(l.jogador.nome, "Teste") == 0, "nome preservado");
    VERIFICAR(l.jogador.pontuacao == 1234 && l.jogador.regiao == 1, "pontos e região");
    VERIFICAR(l.jogador.x == 5 && l.jogador.y == 9, "posição");
    VERIFICAR(l.equipe.qtdAtivos == 3 && l.equipe.qtdArmazenamento == 3, "equipe e armazenamento");
    VERIFICAR(l.equipe.indiceAtivo == 1, "criatura ativa");
    VERIFICAR(l.equipe.ativos[1].hp == 7 && l.equipe.ativos[1].nivel == 8, "atributos da criatura");
    VERIFICAR(strcmp(l.equipe.ativos[1].habilidades[1].nome, "Tempestade de Penas") == 0, "habilidades");
    estadoLiberar(&g);
    estadoLiberar(&l);
    remove(arq);
}

static void ct07_save_corrompido(void)
{
    printf("CT-07 arquivo de save corrompido ou inexistente\n");
    const char *arq = "saves/corrompido.sav";
    EstadoJogo g = novoJogo();
    VERIFICAR(arquivosCarregar("saves/nao_existe.sav", &g) == ARQ_ERRO_ABRIR, "arquivo inexistente");

    FILE *f = fopen(arq, "w");
    fputs("CURSEDSOULS 1\nTeste 0 2 7 0 3 3 0 0 0\n1 0 0\n99 5 0 10 10 5 5 5 1 1\n", f);
    fclose(f);
    VERIFICAR(arquivosCarregar(arq, &g) != ARQ_OK, "espécie inválida rejeitada");
    VERIFICAR(strcmp(g.jogador.nome, "Teste") == 0 && g.equipe.qtdAtivos == 1, "estado original intacto");

    f = fopen(arq, "w");
    fputs("lixo qualquer\n", f);
    fclose(f);
    VERIFICAR(arquivosCarregar(arq, &g) == ARQ_ERRO_FORMATO, "cabeçalho inválido");

    f = fopen(arq, "w");
    fputs("CURSEDSOULS 1\nTeste 9 2 7 0 3 3 0 0 0\n1 0 0\n0 5 0 10 10 5 5 5 1 1\n", f);
    fclose(f);
    VERIFICAR(arquivosCarregar(arq, &g) == ARQ_ERRO_DADOS, "região fora dos limites");

    f = fopen(arq, "w");
    fputs("CURSEDSOULS 1\nTeste 0 2 7 0 3 3 0 0 0\n2 0 0\n0 5 0 10 10 5 5 5 1 1\n", f);
    fclose(f);
    VERIFICAR(arquivosCarregar(arq, &g) != ARQ_OK, "save truncado");
    estadoLiberar(&g);
    remove(arq);
}

static void ct08_mapa(void)
{
    printf("CT-08 mapa: colisão e limites\n");
    for (int r = 0; r < TOTAL_REGIOES; r++) {
        Mapa *m = mapaCriar(r);
        VERIFICAR(m != NULL, "mapa criado");
        if (m == NULL) continue;
        VERIFICAR(mapaCaminhavel(m, SPAWN_X, SPAWN_Y), "spawn caminhável");
        VERIFICAR(mapaCaminhavel(m, SPAWN_X_DIREITA, SPAWN_Y), "spawn direito caminhável");
        VERIFICAR(!mapaCaminhavel(m, 0, 0), "borda é parede");
        VERIFICAR(!mapaCaminhavel(m, -1, 5), "x negativo");
        VERIFICAR(!mapaCaminhavel(m, 5, MAPA_ALTURA), "y acima do limite");
        VERIFICAR(mapaTile(m, 9999, 9999) == '#', "fora dos limites = parede");
        for (int y = 0; y < m->altura; y++) {
            VERIFICAR((int)strlen(m->tiles[y]) == m->largura, "largura da linha");
        }
        mapaLiberar(m);
    }
    VERIFICAR(mapaCriar(-1) == NULL && mapaCriar(TOTAL_REGIOES) == NULL, "região inválida");
    srand(7);
    Mapa *m = mapaCriar(0);
    Criatura c;
    for (int i = 0; i < 50; i++) {
        VERIFICAR(mapaGerarCriatura(m, &c) && c.nivel >= 2 && c.nivel <= 5, "nível dentro da faixa");
    }
    mapaLiberar(m);
    m = mapaCriar(2);
    VERIFICAR(mapaGerarCriatura(m, &c) == 0, "covil não tem encontros aleatórios");
    mapaLiberar(m);
}

static void ct09_experiencia(void)
{
    printf("CT-09 experiência e nível máximo\n");
    Criatura c;
    criaturaCriar(&c, 0, 1);
    int hp1 = c.hpMax;
    VERIFICAR(ganharExperiencia(&c, xpParaProximoNivel(1)) == 1 && c.nivel == 2, "sobe 1 nível");
    VERIFICAR(c.hpMax > hp1, "atributos aumentam");
    ganharExperiencia(&c, 1000000);
    VERIFICAR(c.nivel == NIVEL_MAXIMO, "para no nível máximo");
    VERIFICAR(ganharExperiencia(&c, 100) == 0, "sem ganho no nível máximo");
    VERIFICAR(ganharExperiencia(&c, -5) == 0, "xp negativo ignorado");
    VERIFICAR(!criaturaCriar(&c, 0, 0) && !criaturaCriar(&c, 0, 21), "nível inválido");
}

static void ct10_batalha(void)
{
    printf("CT-10 batalha: ações inválidas e fim de combate\n");
    srand(3);
    EstadoJogo g = novoJogo();
    Criatura inimigo;
    criaturaCriar(&inimigo, 1, 3);
    Batalha b;
    batalhaIniciar(&b, &inimigo);

    g.equipe.ativos[0].energia = 0;
    VERIFICAR(batalhaExecutarTurno(&b, &g, ACAO_HABILIDADE, 0) == 0, "sem energia não gasta turno");
    VERIFICAR(b.turno == 0, "turno não avançou");
    VERIFICAR(batalhaExecutarTurno(&b, &g, ACAO_TROCAR, 0) == 0, "trocar para a própria ativa");
    VERIFICAR(batalhaExecutarTurno(&b, &g, ACAO_ITEM, 0) == 0, "poção com vida cheia");

    inimigo.hp = 1;
    g.equipe.ativos[0].velocidade = 999;
    b.inimigo.hp = 1;
    VERIFICAR(batalhaExecutarTurno(&b, &g, ACAO_ATACAR, 0) == 1, "ataque válido");
    VERIFICAR(b.estado == BAT_VITORIA, "inimigo derrotado = vitória");
    VERIFICAR(g.jogador.pontuacao > 0 && g.jogador.vitorias == 1, "pontos e vitórias");
    VERIFICAR(batalhaExecutarTurno(&b, &g, ACAO_ATACAR, 0) == 0, "batalha encerrada não aceita ações");

    /* derrota: equipe inteira sem vida */
    Criatura forte;
    criaturaCriar(&forte, 4, 10);
    batalhaIniciar(&b, &forte);
    g.equipe.ativos[0].hp = 1;
    g.equipe.ativos[0].velocidade = 0;
    b.inimigo.ataque = 999;
    batalhaExecutarTurno(&b, &g, ACAO_ATACAR, 0);
    VERIFICAR(b.estado == BAT_DERROTA, "sem criaturas vivas = derrota");
    estadoLiberar(&g);
}

static void ct11_nome_e_ranking(void)
{
    printf("CT-11 validação de nome e ranking\n");
    VERIFICAR(validarNome("Gledson_1"), "nome válido");
    VERIFICAR(!validarNome(""), "nome vazio");
    VERIFICAR(!validarNome(NULL), "nome nulo");
    VERIFICAR(!validarNome("com espaco"), "espaço não permitido");
    VERIFICAR(!validarNome("nome;rm"), "caractere especial");
    char longo[80];
    memset(longo, 'a', sizeof longo - 1);
    longo[sizeof longo - 1] = '\0';
    VERIFICAR(!validarNome(longo), "nome longo demais");

    const char *arq = "saves/ranking_teste.txt";
    remove(arq);
    for (int i = 0; i < 13; i++) {
        char jogador[16];
        snprintf(jogador, sizeof jogador, "J%d", i);
        pontuacaoRegistrar(arq, jogador, i * 10);
    }
    RegistroPontuacao v[MAX_RANKING];
    int n = pontuacaoCarregar(arq, v, MAX_RANKING);
    VERIFICAR(n == MAX_RANKING, "ranking guarda só o top 10");
    VERIFICAR(v[0].pontos == 120 && v[n - 1].pontos == 30, "ordem decrescente");
    VERIFICAR(!pontuacaoRegistrar(arq, "ok", -1), "pontuação negativa rejeitada");

    /* o mesmo nome não se repete: vale a melhor pontuação */
    pontuacaoRegistrar(arq, "J12", 50);                       /* menor: não muda nada */
    n = pontuacaoCarregar(arq, v, MAX_RANKING);
    int repetidos = 0;
    for (int i = 0; i < n; i++) if (strcmp(v[i].nome, "J12") == 0) repetidos++;
    VERIFICAR(n == MAX_RANKING && repetidos == 1 && v[0].pontos == 120, "pontuação menor não duplica nem substitui");
    pontuacaoRegistrar(arq, "J12", 500);                      /* maior: substitui */
    n = pontuacaoCarregar(arq, v, MAX_RANKING);
    VERIFICAR(n == MAX_RANKING && v[0].pontos == 500 && strcmp(v[0].nome, "J12") == 0, "pontuação maior substitui");
    pontuacaoRegistrar(arq, "Novo", 5);                       /* abaixo do top 10: fica de fora */
    n = pontuacaoCarregar(arq, v, MAX_RANKING);
    int achouNovo = 0;
    for (int i = 0; i < n; i++) if (strcmp(v[i].nome, "Novo") == 0) achouNovo = 1;
    VERIFICAR(n == MAX_RANKING && !achouNovo, "pontuação abaixo do top 10 não entra");
    remove(arq);
}

static void ct15_guardiao_invocador(void)
{
    printf("CT-15 Guardião invocador: equipe própria e vitória só com todas derrotadas\n");
    srand(5);
    Criatura time[MAX_EQUIPE];
    int n = chefeMontarEquipe(time, MAX_EQUIPE);
    VERIFICAR(n == TAMANHO_EQUIPE_CHEFE, "equipe do chefe montada");
    VERIFICAR(chefeMontarEquipe(time, 0) == 0 && chefeMontarEquipe(NULL, 3) == 0, "parâmetros inválidos");

    EstadoJogo g = novoJogo();
    Batalha b;
    batalhaIniciarTreinador(&b, NOME_CHEFE, time, n);
    VERIFICAR(b.ehChefe && b.qtdTimeInimigo == n && b.indiceInimigo == 0, "batalha de chefe iniciada");
    VERIFICAR(strcmp(b.inimigo.nome, time[0].nome) == 0, "primeira criatura em campo");
    VERIFICAR(batalhaExecutarTurno(&b, &g, ACAO_FUGIR, 0) == 0, "não foge do chefe");

    for (int i = 0; i < n; i++) {
        g.equipe.ativos[0].hp = g.equipe.ativos[0].hpMax;
        g.equipe.ativos[0].velocidade = 999;
        b.inimigo.hp = 1;
        VERIFICAR(batalhaExecutarTurno(&b, &g, ACAO_ATACAR, 0) == 1, "ataque derruba a criatura do chefe");
        if (i < n - 1) {
            VERIFICAR(b.estado == BAT_EM_ANDAMENTO, "batalha continua enquanto há criaturas");
            VERIFICAR(b.indiceInimigo == i + 1 && strcmp(b.inimigo.nome, time[i + 1].nome) == 0,
                      "chefe envia a próxima criatura");
            VERIFICAR(!g.jogador.chefeDerrotado && g.jogador.vitorias == 0, "chefe ainda de pé");
        }
    }
    VERIFICAR(b.estado == BAT_VITORIA, "todas caíram = vitória");
    VERIFICAR(g.jogador.chefeDerrotado == 1 && g.jogador.vitorias == 1, "chefe derrotado registrado");
    VERIFICAR(g.jogador.pontuacao >= BONUS_PONTOS_CHEFE, "bônus de pontos do chefe");
    VERIFICAR(batalhaExecutarTurno(&b, &g, ACAO_ATACAR, 0) == 0, "sem ações após o fim");
    estadoLiberar(&g);
}

static void ct16_lore(void)
{
    printf("CT-16 lore: painéis e índices inválidos\n");
    VERIFICAR(loreQtdPaineis() > 0, "há painéis de história");
    for (int i = 0; i < loreQtdPaineis(); i++) {
        VERIFICAR(strlen(lorePainel(i)) > 0, "painel não vazio");
    }
    VERIFICAR(strcmp(lorePainel(-1), "") == 0, "índice negativo = texto vazio");
    VERIFICAR(strcmp(lorePainel(loreQtdPaineis()), "") == 0, "índice acima do limite = texto vazio");
    VERIFICAR(strlen(loreFalaChefe()) > 0, "fala do chefe existe");
}

static void ct18_save_por_jogador(void)
{
    printf("CT-18 save separado por jogador\n");
    char caminho[64];
    VERIFICAR(arquivosCaminhoSave("Ana", caminho, sizeof caminho) && strcmp(caminho, "saves/Ana.sav") == 0,
              "caminho montado a partir do nome");
    VERIFICAR(!arquivosCaminhoSave("", caminho, sizeof caminho), "nome vazio");
    VERIFICAR(!arquivosCaminhoSave(NULL, caminho, sizeof caminho), "nome nulo");
    VERIFICAR(!arquivosCaminhoSave("../x", caminho, sizeof caminho), "nome com caminho");
    VERIFICAR(!arquivosCaminhoSave("a b", caminho, sizeof caminho), "nome com espaço");
    VERIFICAR(!arquivosCaminhoSave("Ana", caminho, 5), "buffer pequeno demais");

    EstadoJogo a, b, l;
    estadoInicializar(&a);
    estadoInicializar(&b);
    estadoInicializar(&l);
    estadoNovo(&a, "TesteAna", 0);
    estadoNovo(&b, "TesteBeto", 1);
    a.jogador.pontuacao = 111;
    b.jogador.pontuacao = 222;

    char pa[64], pb[64], pn[64];
    arquivosCaminhoSave("TesteAna", pa, sizeof pa);
    arquivosCaminhoSave("TesteBeto", pb, sizeof pb);
    arquivosCaminhoSave("TesteNaoExiste", pn, sizeof pn);
    VERIFICAR(arquivosSalvar(pa, &a) == ARQ_OK && arquivosSalvar(pb, &b) == ARQ_OK, "dois saves gravados");
    VERIFICAR(arquivosCarregar(pa, &l) == ARQ_OK && l.jogador.pontuacao == 111, "carrega o save da Ana");
    VERIFICAR(arquivosCarregar(pb, &l) == ARQ_OK && l.jogador.pontuacao == 222, "carrega o save do Beto");
    VERIFICAR(strcmp(l.jogador.nome, "TesteBeto") == 0, "um save não sobrescreve o outro");
    VERIFICAR(arquivosCarregar(pn, &l) == ARQ_ERRO_ABRIR, "jogador sem save");
    estadoLiberar(&a);
    estadoLiberar(&b);
    estadoLiberar(&l);
    remove(pa);
    remove(pb);
}

int main(void)
{
    ct01_dano_e_afinidade();
    ct02_chances_ressurreicao();
    ct03_tentativa_ressurreicao();
    ct04_limite_equipe();
    ct05_troca_invalida();
    ct06_salvar_carregar();
    ct07_save_corrompido();
    ct08_mapa();
    ct09_experiencia();
    ct10_batalha();
    ct11_nome_e_ranking();
    ct15_guardiao_invocador();
    ct16_lore();
    ct18_save_por_jogador();
    printf("\n%d verificações, %d falha(s)\n", total, falhas);
    return falhas == 0 ? 0 : 1;
}
