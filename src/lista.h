#ifndef LISTA_H_
#define LISTA_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#define TAM_NOME 50
#define TAM_BUFFER 150

struct lista {
    int tam;                // número de membros no diretório
    long int posicao;       // ptr para o diretório
    struct nodo* inicio;    // ptr para o primeiro nodo da lista
    struct nodo* fim;       // ptr para o final da lista
};

struct nodo {
    char nome[TAM_NOME];
    uid_t UID;
    mode_t permissoes;
    off_t tam;
    long int ultimaMod;
    int ordemArq;
    char local[TAM_BUFFER];
    struct nodo* proximo;
};

// insere o novo nodo na lista de membros 
void inserirNodo (struct lista* diretorio, struct nodo* novo);

// cria um novo ndo, preenchendo suas informações [nome, tamanho, datas, etc]
struct nodo* criarNodo (struct nodo* novo, FILE* arq, char* nome);

// desaloca a memória da lista
void destruirLista (struct lista* diretorio);

#endif