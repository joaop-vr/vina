#include "lista.h"
#include "excluir.h"
#include "complementares.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX 1024

void excluir (char *argv[], struct lista* diretorio) {

    int indice;

    FILE* archive = fopen(argv[2], "r+");
    if (archive == NULL) {
        printf("Erro ao abrir %s.\n", argv[2]);
        exit (1);
    }

    if(ehVazio(archive) == 0) {
        printf("Archive está vazio! Não há o que excluir.\n");
    }
    else {

        indice = 3;

        /* chama a função escluirMembro() para cada arquivo passado 
        na linha de comando */
        while (argv[indice]) {

            char* nome = strdup(argv[indice]);
            excluirMembro(archive, diretorio, argv[2], nome);
            indice++;
            free(nome);
        }
    }
    fclose(archive);
    
}

void excluirMembro (FILE* archive, struct lista* diretorio, char* argv2, char* nome) {

    int N;
    off_t novoTam;
    long int posicaoDiretorio, proxLeitura, proxEscrita, limite;
    struct nodo* retorno;
    char* buffer = malloc(1024*sizeof(char));

    // lê o num de membros e a posição do diretorio 
    lerCabeca(archive, &N, &posicaoDiretorio);
    diretorio->posicao = posicaoDiretorio;

    /* avança para a posição do diretório e 
    guarda seu conteúdo se ele não já estiver inicializado */
    fseek(archive, diretorio->posicao, SEEK_SET);
    if (diretorio->inicio == NULL) 
        guardarDiretorio(archive, diretorio);

    // procura pelo arquivo no diretório
    retorno = buscar(diretorio, nome);

    if (retorno == NULL) {
        printf("O arquivo '%s' não está presente no 'archiver'.\n", nome);
    }
    else {
        
        // descartando o diretório do archive
        novoTam = totalBytes(diretorio);
        truncate(argv2, novoTam);

        if (diretorio->tam == 1) {
            /* Excluindo o ultimo membro */
            long int novoEOF = obterPosicao(archive, diretorio, retorno->ordemArq);
            truncate(argv2, novoEOF);
        }
        else {

            fseek(archive, 0, SEEK_END);
            limite = ftell(archive);

            fseek(archive, 0, SEEK_SET); 

            // proxEscrita: ptr para o início do conteúdo do arquivo a ser excluído
            // proxLeitura: ptr para o início do conteúdo do aquivo que vem logo em seguida no archive
            proxEscrita = obterPosicao(archive, diretorio, retorno->ordemArq);
            proxLeitura = obterPosicao(archive, diretorio, retorno->ordemArq+1);

            fseek(archive, proxLeitura, SEEK_SET);
            while (proxLeitura <= limite) {
                
                memset(buffer, 0, MAX);
                fread(buffer, sizeof(char), MAX, archive);
                proxLeitura = ftell(archive);
                
                fseek(archive, proxEscrita, SEEK_SET);
                fwrite(buffer, sizeof(char), MAX, archive);
                proxEscrita = ftell(archive);

                fseek(archive, proxLeitura, SEEK_SET);
            }

            // descartando o espaço ocupado pelo arquivo excluído
            novoTam = calcularTruncate(diretorio, retorno->ordemArq);
            truncate(argv2, novoTam);

        }

        // removendo o nodo que continha meta dados do arquivo excluído
        excluirNoDiretorio (diretorio, nome);
    }
    
    /* evitando que essas funções sejam chamadas caso "busca()" retorne NULL,
    ou seja, não foi encontrado*/
    if (retorno != NULL) {

        fseek(archive, 0, SEEK_END);
        diretorio->posicao = ftell(archive);

        escreverDiretorio(archive, diretorio);

        atualizarCabeca(archive, diretorio);
    }

    free(buffer);
}

void excluirNoDiretorio (struct lista* diretorio, char* nome) {

    struct nodo* excluir, *aux, *anterior;
    int i, ordemExcluida, temHierarquia;
    char* ptrNome;
    char buffer[TAM_BUFFER];

    /* trata as diferentes mandeiras 
    de como o nome do arquivo foi passado*/
    temHierarquia = 0;
    ptrNome = NULL;
    ptrNome = strrchr(nome, '/');
    strcat(buffer, ""); ///// AQUIIIIIII
    if (ptrNome != NULL)
        temHierarquia = 1;     // identifica se nome do arquivo foi passado com caminho

    if (temHierarquia == 0) {
        strcpy(buffer, "./");
        strcat(buffer, nome);
    }
    else if(nome[0] == '/') {
        strcpy(buffer, ".");
        strcat(buffer, nome);
    }
    else if(nome[0] != '.') {
        strcpy(buffer, "./");
        strcat(buffer, nome);
    }
    else {
        strcpy(buffer, nome);
    }
        
    i = 0;
    excluir = NULL;
    anterior = NULL;

    // obtendo o nodo anterior em relação que desejamos excluir
    for (aux = diretorio->inicio; i < diretorio->tam; aux = aux->proximo) {
        if (strcmp(aux->local, buffer) == 0)
            excluir = aux;
        if (excluir == NULL)
            anterior = aux;
        i++;
    }

    if (excluir == NULL) {
        printf("O arquivo %s nao esta no archive\n", nome);
        exit(1);
    }
    else {

        diretorio->tam--;
        ordemExcluida = excluir->ordemArq;

        if(excluir == diretorio->inicio) {          // caso de exclusão no primeiro nodo
            diretorio->inicio = excluir->proximo;
        }
        else if(excluir == diretorio->fim) {        // caso de exclusão no último nodo
            diretorio->fim = anterior;
            diretorio->fim->proximo = NULL;
        }
        else {
            anterior->proximo = excluir->proximo;
        }
        
        free(excluir);

        ajustarOrdem(diretorio, ordemExcluida);
    }
}