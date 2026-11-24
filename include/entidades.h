#ifndef ENTIDADES_H
#define ENTIDADES_H

#define MAX_NOME 32
#define MAX_HABILIDADES 2
#define NIVEL_MAXIMO 20
#define TOTAL_ESPECIES 7
#define NOME_CHEFE "Guardião Corrompido"   /* o chefe é um invocador, não uma criatura */
#define TAMANHO_EQUIPE_CHEFE 3

typedef enum { AFIN_SOMBRIA, AFIN_FOGO, AFIN_GELO, AFIN_CAOS, AFIN_TOTAL } Afinidade;
typedef enum { RAR_COMUM, RAR_INCOMUM, RAR_RARA, RAR_EPICA, RAR_LENDARIA, RAR_TOTAL } Raridade;
typedef enum { EFEITO_NENHUM, EFEITO_REDUZ_DEFESA, EFEITO_DRENAR } EfeitoHabilidade;

typedef struct {
    char nome[MAX_NOME];
    int poder;
    int custoEnergia;
    EfeitoHabilidade efeito;
} Habilidade;

typedef struct {
    int especieId;
    char nome[MAX_NOME];
    Afinidade afinidade;
    Raridade raridade;
    int nivel, xp;
    int hp, hpMax;
    int ataque, defesa, velocidade;
    int energia, energiaMax;
    int modDefesa;                       /* penalidade temporária (não é salva) */
    Habilidade habilidades[MAX_HABILIDADES];
} Criatura;

#define DIR_BAIXO 0
#define DIR_CIMA 1
#define DIR_ESQUERDA 2
#define DIR_DIREITA 3

typedef struct {
    char nome[MAX_NOME];
    int regiao, x, y;
    int pontuacao;
    int correntes;                       /* cargas de "Correntes da Maldição" */
    int pocoes;
    int vitorias;
    int conquistadas;
    int chefeDerrotado;
    int direcao;                         /* para onde olha: DIR_* (só visual, não é salvo) */
    int passo;                           /* conta os passos; escolhe o quadro da animação de andar */
} Jogador;

int criaturaCriar(Criatura *c, int especieId, int nivel);
int criaturaSubirNivel(Criatura *c);
int criaturaEstaViva(const Criatura *c);
int criaturaDefesaEfetiva(const Criatura *c);
void criaturaRestaurar(Criatura *c);
int multiplicadorAfinidade(Afinidade atacante, Afinidade defensor);   /* em % */
const char *nomeAfinidade(Afinidade a);
const char *nomeRaridade(Raridade r);
/* Monta a equipe do chefe em saida[]; retorna quantas criaturas foram criadas. */
int chefeMontarEquipe(Criatura *saida, int max);

#endif
