#include "pontuacao.h"
#include "arquivos.h"
#include "regras.h"
#include <stdio.h>
#include <string.h>

/* Ordena do maior para o menor (método da bolha). */
static void ordenar(RegistroPontuacao *v, int n)
{
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (v[j].pontos < v[j + 1].pontos) {
                RegistroPontuacao tmp = v[j];
                v[j] = v[j + 1];
                v[j + 1] = tmp;
            }
        }
    }
}

int pontuacaoCarregar(const char *caminho, RegistroPontuacao *vetor, int maximo)
{
    if (caminho == NULL || vetor == NULL || maximo <= 0) return 0;
    FILE *f = fopen(caminho, "r");
    if (f == NULL) return 0;

    int n = 0, pontos;
    char nome[MAX_NOME];
    while (n < maximo && fscanf(f, "%d %31s", &pontos, nome) == 2) {
        if (pontos < 0) continue;
        vetor[n].pontos = pontos;
        snprintf(vetor[n].nome, sizeof vetor[n].nome, "%s", nome);
        n++;
    }
    fclose(f);
    ordenar(vetor, n);
    return n;
}

int pontuacaoRegistrar(const char *caminho, const char *nome, int pontos)
{
    RegistroPontuacao v[MAX_RANKING + 1];
    if (caminho == NULL || !validarNome(nome) || pontos < 0) return 0;

    int n = pontuacaoCarregar(caminho, v, MAX_RANKING);

    /* cada nome aparece uma vez só, com a sua melhor pontuação */
    int achou = 0;
    for (int i = 0; i < n; i++) {
        if (strcmp(v[i].nome, nome) == 0) {
            achou = 1;
            if (pontos > v[i].pontos) v[i].pontos = pontos;
        }
    }
    if (!achou) {
        snprintf(v[n].nome, sizeof v[n].nome, "%s", nome);
        v[n].pontos = pontos;
        n++;
    }
    ordenar(v, n);
    if (n > MAX_RANKING) n = MAX_RANKING;    /* só ficam os 10 melhores */

    if (!arquivosGarantirDiretorio(caminho)) return 0;
    FILE *f = fopen(caminho, "w");
    if (f == NULL) return 0;
    for (int i = 0; i < n; i++) fprintf(f, "%d %s\n", v[i].pontos, v[i].nome);
    return fclose(f) == 0;
}
