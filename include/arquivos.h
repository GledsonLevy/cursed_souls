#ifndef ARQUIVOS_H
#define ARQUIVOS_H

#include <stddef.h>
#include "regras.h"

typedef enum {
    ARQ_OK,
    ARQ_ERRO_ABRIR,
    ARQ_ERRO_FORMATO,
    ARQ_ERRO_DADOS
} ResultadoArquivo;

ResultadoArquivo arquivosSalvar(const char *caminho, const EstadoJogo *g);
/* Em caso de erro, *g permanece intacto. g deve estar inicializado. */
ResultadoArquivo arquivosCarregar(const char *caminho, EstadoJogo *g);
int arquivosExiste(const char *caminho);
/* Monta "saves/<nome>.sav" (cada jogador tem o seu save). Retorna 0 se o nome for inválido. */
int arquivosCaminhoSave(const char *nome, char *saida, size_t tam);
int arquivosGarantirDiretorio(const char *caminho);   /* cria a pasta do arquivo */

#endif
