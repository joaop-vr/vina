#ifndef EXCLUIR_H_
#define EXCLUIR_H_

#include "lista.h"

// função principal de exclusão. a partir dela chamamos as demais funções relacionadas a esse processo
void excluir (char* argv[], struct lista* diretorio);

// exclui o conteúdo do Membro no archive
void excluirMembro (FILE* archive, struct lista* diretorio, char* argv2, char* nome);

// realiza a exclusão do membro na lista encadeada
void excluirNoDiretorio (struct lista* diretorio, char* nome);

#endif