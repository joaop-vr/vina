#ifndef EXTRAIR_H_
#define EXTRAIR_H_

#include "lista.h"

// função principal de extração. a partir dela chamamos as demais funções relacionadas a esse processo
void extrair (char* argv[], int argc, struct lista* diretorio);

// extrai o membro do archive (tanto o conteúdo quanto os meta dados)
void extrairMembro (char* argv2, struct lista* diretorio, char* nome);

// cria o arquivo que se deseja extrair e recria hierarquia de diretório se necessário
void criarArquivo (struct nodo* alvo, char* caminhoArq);

// transcreve o conteúdo do Membro dentro do archive para o arquivo .txt criado
void transcreverMembro (FILE* archive, struct lista* diretorio, struct nodo* alvo);

#endif