#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <dirent.h>

#include "lista.h"

#define TAM_BUFFER 150


void inserirNodo (struct lista* diretorio, struct nodo* novo) {

    /*fila vazia*/
    if (diretorio->inicio == NULL) {
        novo->proximo = diretorio->inicio;
        diretorio->inicio = novo;
        diretorio->fim = novo;
        diretorio->tam++;
    }
    else {
        /*Inserindo no fim*/
        diretorio->fim->proximo = novo;
        diretorio->fim = novo;
        novo->proximo = NULL;
        diretorio->tam++;
    }

}

struct nodo* criarNodo (struct nodo* novo, FILE* arq, char* nome) {

    int descritor, temHierarquia;
    struct stat metaDados;
    char* ptrNome;
    char buffer[TAM_BUFFER];

    //Obtém o descritor de arquivo usando fileno() 
    descritor = fileno(arq); 
    if (fstat(descritor, &metaDados) < 0) {
        perror("Erro ao obter informações do arquivo");
        exit (1);
    }

    // Inicializando o identificador de hirarquia de diretório com 0
    //(ou seja, não possui hierarquia)
    temHierarquia = 0;

    ptrNome = NULL;
    ptrNome = strrchr(nome, '/');

    if (ptrNome != NULL)
        temHierarquia = 1;     // identifica se nome do arquivo foi passado com caminho

    /* Guarda o nome do arquivo na struct */
    if (temHierarquia == 0) {
        strcpy(novo->nome, nome);
    }
    else {
        ptrNome++;          // Incrementa para pular o caractere '/'
        strcpy(novo->nome, ptrNome);
    }

    /* Guardando UID, permissoes, tamanho e
    data de ultima modificacao */
    novo->UID = metaDados.st_uid;
    novo->permissoes = metaDados.st_mode;
    novo->tam = metaDados.st_size;
    novo->ultimaMod = metaDados.st_mtime;

    /* Trata as diferentes mandeiras 
    de como o nome do arquivo foi passado*/
    if (temHierarquia == 0) {
        strcpy(buffer, "./");
        strcat(buffer, nome);
        strcpy(novo->local, buffer);
    }
    else if(nome[0] == '/') {
        strcpy(buffer, ".");
        strcat(buffer, nome);
        strcpy(novo->local, buffer);
    }
    else if(nome[0] != '.') {
        strcpy(buffer, "./");
        strcat(buffer, nome);
        strcpy(novo->local, buffer);
    }
    else
        strcpy(novo->local, nome); 
    
    return novo;
}

void destruirLista (struct lista* diretorio) {

    struct nodo *atual, *proximo;
    atual = diretorio->inicio;
    while (atual != NULL) {
        proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }
    
    free(diretorio);
}