#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "extrair.h"
#include "excluir.h"
#include "complementares.h"

#define MAX 1024

void extrair (char* argv[], int argc, struct lista* diretorio) {

    int N, indice, cont;
    long int posicaoDiretorio;
    struct nodo* aux, *prox;

    FILE* archive = fopen(argv[2], "r+");
    if (!archive) {
        printf("[Função extrair]\n");
        printf("Erro: não foi possível abrir o arquivo %s\n", argv[2]);
        exit(1);
    }

    if (ehVazio(archive) == 0) {
        printf("Archive está vazio! Não há o que extrair.\n");
    }
    else {

        // lê o num de membros e a posição do diretorio 
        lerCabeca(archive, &N, &posicaoDiretorio);

        // desloca para a posição do diretório para guardá-lo na lista
        fseek(archive, posicaoDiretorio, SEEK_SET);
        guardarDiretorio(archive, diretorio);

        indice = 3; 
        if (argc == 3) { // codigo para extrair todos

            cont = 0;
            aux = diretorio->inicio;
            while (cont < N) {

                prox = aux->proximo;
                char* buffer = aux->local;  // adaptando o nome que estava guardado como relativo
                if ((aux->local[0] != '.') && (aux->local[1] != '.'))
                    buffer += 2;

                extrairMembro(argv[2], diretorio, buffer);

                aux = prox;
                cont++;
            }
            
        }
        else {
            while(argv[indice]) { // extrai os arquivos que foram passados pelo usuário
                extrairMembro(argv[2], diretorio, argv[indice]);
                indice++;
            }
        }
    }
    fclose(archive);
}

void extrairMembro (char* argv2, struct lista* diretorio, char* nome) {

    struct nodo* retorno = NULL;
    char dirAtual[1024];

    FILE* archive = fopen(argv2, "r+");
    if (!archive) {
        printf("Não foi possível abrir %s.\n", argv2);
        exit(1);
    }

    // retorno recebe ptr para o nodo que deseja buscar
    retorno = buscar(diretorio, nome);

    if (retorno == NULL) {
        printf("Erro: o arquivo %s não está no 'archiver'.\n", nome);
    }
    else {

        // dirAtual guarda o diretório atual, para que posteriormente possamos retornar
        getcwd(dirAtual, sizeof(dirAtual));

        // criando o arquivo novo, juntamente com a hierarquia de diretorios
        criarArquivo(retorno, nome);        

        // transcreve o conteúdo do arquivo dentro do archive para o arquivo recem criado
        transcreverMembro(archive, diretorio, retorno);

        // retornando ao diretório de origem do programa
        chdir(dirAtual);

        // exclui o membro extraído
        excluirMembro(archive, diretorio, argv2, nome);
    }

    fclose(archive);
}

void criarArquivo (struct nodo* alvo, char* nome) {

    int temHierarquia, descritor;
    char *ptrNome, *token, *dirAtual, caminhoArq[TAM_BUFFER], buffer[TAM_BUFFER];
    struct stat st;

    strcpy(caminhoArq, nome);

    temHierarquia = 0;
    ptrNome = NULL;
    ptrNome = strrchr(caminhoArq, '/');

    // identifica se nome do arquivo foi passado com caminho
    if (ptrNome != NULL)
        temHierarquia = 1;     

    // tratando os diferentes modos como o arquivo foi passado
    // (com/sem caminho, caminho relativo/absoluto)
    if (temHierarquia == 0) {
        strcpy(buffer, caminhoArq);
    }
    else {
        if (ptrNome != NULL) {
            ptrNome++;          // incrementa para pular o caractere '/'
            strcpy(buffer, ptrNome);
        }
        else {
            printf("Erro: a função strrchr() retornou um ptr inválido.\n");
            exit(1);
        }
    }

    // recriando hierarquia de diretorios se o arquivo não está no diretorio atual
    if (temHierarquia == 1) {
        ptrNome = strrchr(caminhoArq, '/');
        ptrNome++;
        if (access(ptrNome, F_OK) != 0) {

            ptrNome = strrchr(caminhoArq, '/');
            *ptrNome = '\0';
            token = strtok(caminhoArq, "./");
            while (token != NULL) {
                
                dirAtual = token;

                if (stat(dirAtual, &st) == 0 && S_ISDIR(st.st_mode)) {
                    // o dirAtual já existe, vamos acessá-lo
                    if (chdir(dirAtual) != 0) {
                        printf("Erro: não foi possível acessar do diretório %s, durante a reconstruição da hierarquia.\n", dirAtual);
                        exit(1);
                    }
                }
                else {
                    if (mkdir(dirAtual, 0700) == 0) {
                        // criando o dirAtual e acessando-o
                        if (chdir(dirAtual) != 0) {
                            printf("Erro: não foi possível acessar do diretório %s, durante a reconstruição da hierarquia.\n", dirAtual);
                            exit(1);
                        }
                    }
                    else 
                        printf("Não foi possível criar o diretório %s.\n", dirAtual);
                }

                token = strtok (NULL, "./");
            }
        }    
    }
    

    // criando o arquivo e repassando UID e permissões
    descritor = open(buffer, O_CREAT | O_WRONLY, alvo->permissoes);
    if (descritor < 0) {
        printf("Erro ao criar o arquivo %s.\n", buffer);
        exit(1);
    }
    fchown(descritor, alvo->UID, -1);
    
    close(descritor);

}

void transcreverMembro (FILE* archive, struct lista* diretorio, struct nodo* alvo) {

    long int proxLeitura, limite;
    char* buffer = malloc(1024*sizeof(char));

    FILE* novoARQ = fopen(alvo->nome, "r+");
    if (!novoARQ) {
        printf("Erro: não foi possível abrir o arquivo %s.\n", alvo->nome);
        exit(1);
    }

    // obtém a posição do início do conteúdo de 'alvo' no archive
    proxLeitura = obterPosicao(archive, diretorio, alvo->ordemArq);

    // limite recebe o final do archive
    fseek(archive, 0, SEEK_END);
    limite = ftell(archive);

    // posiciona o ptr para o início de novoARQ
    fseek(novoARQ, 0, SEEK_SET);
    fseek(archive, proxLeitura, SEEK_SET);

    // transcreve o conteúdo do archive para novoARQ
    while (proxLeitura < limite) {
        
        memset(buffer, 0, MAX);
        fread(buffer, sizeof(char), MAX, archive);
        proxLeitura = ftell(archive);

        fwrite(buffer, sizeof(char), MAX, novoARQ);
        fflush(novoARQ);
    }
    
    // descarta os conteúdos que não correspondem ao arquivo que deseja extrair
    truncate(alvo->nome, alvo->tam);
    
    free(buffer);
    fclose(novoARQ);
}