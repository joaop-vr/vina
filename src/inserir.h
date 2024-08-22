#ifndef INSERIR_H_
#define INSERIR_H_

#include "lista.h"

// função principal de inserção. a partir dela chamamos as demais funções relacionadas a esse processo
void inserir (char* argv[], struct lista* diretorio, int mode);

// utilizada para inciar o archiver, ou seja, realizar a primeir a inserção
void iniciarArchive (char* argv[], FILE* archive, struct lista* diretorio);

// insere o conteúdo do membro no archive, juntamente com os seus meta dados (no diretório)
void inserirMembro (char* argv[], struct lista* diretorio, char* alvoArgv, char*);

// substitui o conteúdo do membro no archive, juntamente com os seus meta dados (no diretório)
void substituirMembro (char* argv[], struct lista* diretorio, char* alvoArgv);

// escreve o conteúdo do membro no archive 
void escreverMembro (FILE* archive, FILE* arq);

#endif