#include "lore.h"

/* Para mudar a história, edite apenas esta tabela. O "\n" quebra a linha. */
static const char *PAINEIS[] = {
    "Há muito tempo, o mundo vivia em equilíbrio\n"
    "com as criaturas que o habitavam. Então uma\n"
    "corrupção misteriosa começou a se espalhar\n"
    "pelas florestas e pelas ruínas.",

    "Por onde ela passa, criaturas comuns se\n"
    "tornam seres amaldiçoados: olhos vermelhos,\n"
    "fome sem fim e uma fúria que ninguém\n"
    "consegue acalmar.",

    "Você nasceu com um dom raro e perigoso: ao\n"
    "derrotar uma criatura, pode recuperar a\n"
    "essência que se desprende dela e trazê-la\n"
    "de volta como um Eco Amaldiçoado.",

    "Nem toda essência aceita voltar. Quanto\n"
    "mais rara a criatura, mais difícil é\n"
    "ressuscitá-la. Mas cada eco conquistado\n"
    "luta ao seu lado e fica mais forte.",

    "No fundo do Covil, o Guardião Corrompido\n"
    "espera. Ele também comanda essências, e a\n"
    "corrupção parece obedecer a ele. Só outro\n"
    "invocador pode detê-lo."
};

#define QTD_PAINEIS ((int)(sizeof PAINEIS / sizeof PAINEIS[0]))

int loreQtdPaineis(void)
{
    return QTD_PAINEIS;
}

const char *lorePainel(int indice)
{
    if (indice < 0 || indice >= QTD_PAINEIS) return "";
    return PAINEIS[indice];
}

const char *loreFalaChefe(void)
{
    return "Mais um invocador chega\n"
           "ao meu covil... Vejamos se\n"
           "as suas essências aguentam\n"
           "as minhas.";
}
