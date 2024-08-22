#include "lista.h"
#include "inserir.h"
#include "excluir.h"
#include "complementares.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX 1024

void inserir (char* argv[], struct lista* diretorio, int mode) {

    int indice, N;
    long int posicaoDiretorio;
    struct nodo* retorno;
    struct stat informacoes;

    FILE* archive = fopen(argv[2], "r+");
    if (!archive) {
        archive = fopen(argv[2], "w+");
    }

    if (ehVazio(archive) == 0) { //se archiver estiver vazio chama-se a função para iniciá-lo
        iniciarArchive(argv, archive, diretorio); 
    }
    else {

        //lê do archive o num atual de membros e a posição para o diretório
        lerCabeca(archive, &N, &posicaoDiretorio);
        diretorio->posicao = posicaoDiretorio;

        fseek(archive, posicaoDiretorio, SEEK_SET);
        guardarDiretorio(archive, diretorio); //guarda o conteúdo do diretorio em uma lista encadeada

        indice = 3;
        while (argv[indice]) {

            lerCabeca(archive, &N, &posicaoDiretorio);

            diretorio->posicao = posicaoDiretorio;
            fseek(archive, posicaoDiretorio, SEEK_SET);

            char* nome = NULL;
            nome = strdup(argv[indice]);
            retorno = buscar(diretorio, nome); //retorna *nodo se achou, null caso contrario

            //verifica se "nome" eh um arquivo válido
            if ((stat(nome, &informacoes) == 0) && (S_ISREG(informacoes.st_mode))) {

                // verifica se há duplicadas dentro do arquivo
                if (retorno == NULL) {  
                    inserirMembro(argv, diretorio, argv[indice], nome);
                }
                else {

                    //[mode 1]: comando do tipo inserir -a
                    if (mode == 1) { 

                        //checando se a versão arquivada é mais recente ou não
                        if (checarRecente(argv[indice], retorno) == 1)
                            substituirMembro(argv, diretorio, argv[indice]);
                        else
                            printf("O arquivo %s não é mais recente do que o arquivado\n", nome);
                    }
                    else {
                        //[mode 0]: comando do tipo inserir -i
                        substituirMembro(argv, diretorio, argv[indice]);
                    }
                        
                }
            }
            else
                printf("Erro: '%s' não eh um arquivo.\n", nome);

            free(nome);
            indice++;
        }
    }
    
    fclose(archive);
}

void iniciarArchive (char* argv[], FILE* archive, struct lista* diretorio) {

    int indice, N;
    long int posicaoDiretorio;
    struct nodo* retorno;
    struct stat informacoes;

    /*Iniciando a cabeca do archiver [cabeça: local onde será 
    escrito a quantidade de membros e a posição para o diretório]*/
    N = 0;
    posicaoDiretorio = 0;
    fwrite(&N, sizeof(int), 1, archive);
    fwrite(&posicaoDiretorio, sizeof(long int), 1, archive);

    indice = 3;
    while (argv[indice]) {

        FILE* arq = fopen(argv[indice], "r");
        if (! arq) {
            printf("Erro, nao foi possivel abrir o arquivo %s\n", argv[indice]);
        }
        else {
            char* nome = NULL;
            nome = strdup(argv[indice]);
            retorno = buscar(diretorio, nome); //retorna *nodo se achou, null caso contrario

            //verifica se "nome" eh um arquivo válido
            if ((stat(nome, &informacoes) == 0) && (S_ISREG(informacoes.st_mode))) {

                // verifica se há duplicadas dentro do arquivo
                if (retorno == NULL) {  

                    escreverMembro(archive, arq); //escreve o conteúdo do arquivo no archiver

                    atualizarDiretorio(diretorio, arq, nome, N); //cria e insere o novo nodo na lista encadeada
                    
                    N++;
                }            
            }
            else
                printf("Erro: '%s' não eh um arquivo.\n", nome);

            free(nome);  
            fclose(arq);
        }
        
        indice++;
    }

    diretorio->posicao = ftell(archive);
    escreverDiretorio(archive, diretorio); //escreve no archiver o conteúdo do diretório
        
    atualizarCabeca(archive, diretorio); //atualiza o num de membros e a posição do diretório

}

void inserirMembro (char* argv[], struct lista* diretorio, char* alvoArgv, char* nome) {

    int N;
    long int posicaoDiretorio;

    FILE* archive = fopen(argv[2], "r+");
    if (archive == NULL) {
        printf("Erro ao abrir %s.\n", argv[2]);
        exit (1);
    }

    FILE* arq = fopen(alvoArgv, "r");
    if (! arq) {
        printf("Erro, nao foi possivel abrir o arquivo %s\n", alvoArgv);
    }
    else {
        // lendo cabeça no archive (num de membros e posição do diretório)
        lerCabeca(archive, &N, &posicaoDiretorio);

        // descartando o diretório do archiver
        posicaoDiretorio = totalBytes(diretorio);
        truncate(argv[2], posicaoDiretorio);

        // escreveu o conteúdo do arq no archiver
        fseek(archive, 0, SEEK_END);
        escreverMembro(archive, arq);
        fflush(archive);

        // adicionando os meta-dados do arquivo novo
        // no diretorio
        atualizarDiretorio(diretorio, arq, nome, N);

        // atualizando ptr para o Diretorio
        fseek(archive, 0, SEEK_END);
        posicaoDiretorio = ftell(archive);
        diretorio->posicao = posicaoDiretorio;

        // transcrevendo o diretorio para o archive
        escreverDiretorio(archive, diretorio);
        fflush(archive);

        // atualizando a cabeça [numero de membros e
        // ponteiro para o Diretorio]
        atualizarCabeca(archive, diretorio);
        fflush(archive);

        fclose(arq);
    }

    fclose(archive);
}

void substituirMembro (char* argv[], struct lista* diretorio, char* alvoArgv) {

    int N;
    long int posicaoDiretorio;
    char* nome = NULL;

    FILE* archive = fopen(argv[2], "r+");
    if (! archive) {
        printf("Erro, nao foi possivel abrir o arquivo %s\n", argv[2]);
        free(nome);
        exit(1);
    }

    FILE* alvoSubstituir = fopen(alvoArgv, "r");
    if (! alvoSubstituir) {
        printf("Erro, nao foi possivel abrir o arquivo %s\n", alvoArgv);
        free(nome);
        exit(1);
    }

    nome = strdup(alvoArgv);

    // lendo cabeça no archive
    lerCabeca(archive, &N, &posicaoDiretorio);
    diretorio->posicao = posicaoDiretorio;

    // excluindo o membro repetido no archive
    excluirMembro(archive, diretorio, argv[2], nome);

    // Calcula o ponteiro para o inicio
    // do arquivo substituído e o escreve no archive
    posicaoDiretorio = totalBytes(diretorio);
    truncate(argv[2], posicaoDiretorio);
    fseek(archive, 0, SEEK_END);
    
    // escreve o conteudo do arquivo substituto no archive
    escreverMembro(archive, alvoSubstituir);
    fflush(archive);

    // adiciona os meta dados do arquivo novo no diretorio
    atualizarDiretorio(diretorio, alvoSubstituir, nome, N);

    fseek(archive, 0, SEEK_END);
    posicaoDiretorio = ftell(archive);
    diretorio->posicao = posicaoDiretorio;

    // escreve o conteúdo do diretorio no archive
    escreverDiretorio(archive, diretorio);
    fflush(archive);

    // atualiza o num de membros e a posicao do diretorio
    atualizarCabeca(archive, diretorio);

    fclose(archive);
    fclose(alvoSubstituir);
    free(nome);
   
}

void escreverMembro (FILE* archive, FILE* arq) {

    char* aux = malloc(1024*sizeof(char));
    size_t bytes_lidos;
    
    // Lê o1024 blocos de dados do arquivo de origem
    bytes_lidos = fread(aux, sizeof(char), MAX, arq);    

    // Enquanto houver dados no bloco
    while (bytes_lidos > 0) {

        // Escreve o bloco de dados no arquivo de destino
        fwrite(aux, sizeof(char), bytes_lidos, archive);
        
        // Lê o próximo bloco de dados do arquivo de origem
        bytes_lidos = fread(aux, sizeof(char), MAX, arq);
    }

    free(aux);
}