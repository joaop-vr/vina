#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#include "inserir.h"
#include "excluir.h"
#include "extrair.h"
#include "complementares.h"
#include "lista.h"

#define MAX 1024
#define TAM_BUFFER 150

struct lista* gerarDiretorio() {

    struct lista* diretorio = malloc(sizeof(struct lista));
    diretorio->tam = 0;
    diretorio->inicio = NULL;
    diretorio->fim = NULL;

    return diretorio;
}

void operacoes (int argc, char *argv[], struct lista* diretorio) {

    int opcao;
        opcao = getopt(argc, argv, "i:a:m:x:r:c:h");

        if ((argc >= 3) && (strlen(argv[1]) == 2)) {
            switch(opcao) {

                case 'i': inserir(argv, diretorio, 0); break;

                case 'a': inserir(argv, diretorio, 1); break;

                case 'm': mover(argv, diretorio); break;

                case 'x': extrair(argv, argc, diretorio); break;

                case 'r': excluir(argv, diretorio); break;
                
                case 'c': imprimir(argv, diretorio); break;

                case 'h':
                    printf("Opcoes de comando disponiveis: \n");
                    printf("    -i : insere/acrescenta um ou mais membros ao archive.\n");
                    printf("    -a : mesmo comportamento da opção -i, mas a substituição de um membro existente ocorre APENAS caso o parâmetro seja mais recente que o arquivado.\n");          
                    printf("    -m target : move o membro indicado na linha de comando para imediatamente depois do membro target existente em archive.\n");
                    printf("    -x : extrai os membros indicados de archive. Se os membros não forem indicados, todos devem ser extraídos.\n");
                    printf("    -r : remove os membros indicados de archive.\n");            
                    printf("    -c : lista o conteúdo de archive em ordem, incluindo as propriedades de cada membro (nome, UID, permissões, tamanho e data de modificação)e sua ordem no arquivo.\n");
                    printf("    -h : imprime uma pequena mensagem de ajuda com as opções disponíveis e encerra.\n");
                    break;

                default:
                    printf("Erro: opcao invalida!\n");
                    break;
            }
        }
        else {
            printf("Erro: Linha de comando errada!\n");
            printf("Espera-se: vina++ <opção> <archive> [membro1 membro2 ...]\n");
        }
}

long int ehVazio (FILE* arq) {

    // verifica se o archiver está vazio

    long int tam = ftell(arq);

    char aux[MAX];
    size_t bytesLidos;
    
    // Lê o primeiro bloco de dados do arquivo
    bytesLidos = fread(aux, sizeof(char), MAX, arq);

    while (bytesLidos > 0) {

        tam = ftell(arq);
        
        // Lê o próximo bloco de dados do arquivo
        bytesLidos = fread(aux, sizeof(char), MAX, arq);
    }

    return tam;
}

int checarRecente (char* argvAlvo, struct nodo* arquivado) {

    int i, descritor, dia1, dia2, mes1, mes2, horas1, horas2,
    minutos1, minutos2, seg1, seg2, ehRecente;
    struct stat metaDados;
    char *nome1, *token;
    struct tm *timeInfo;
    char buffer[80], auxmes[5];
    long int auxTime;

    nome1 = strdup(argvAlvo);
    FILE* arq1 = fopen(argvAlvo, "r");
    if(!arq1) {
        printf("ERRO: não foi possível abrir %s.\n", nome1);
        exit(1);
    }
    
    // guardando a data e hora da última modificação dos arquivos que compararemos
    for (i = 0; i < 2; i++) {

        auxTime = arquivado->ultimaMod;

        if (i == 0) {
            descritor = fileno(arq1);
        
            if (fstat(descritor, &metaDados) < 0) {
                printf("Erro ao obter informações do arquivo %s.\n", nome1);
                exit (1);
            }
            auxTime = metaDados.st_mtime;
        }

        timeInfo = localtime(&auxTime);
        strftime(buffer, sizeof(buffer), "%b %e %H:%M:%S", timeInfo);

        token = strtok(buffer, " :");
        strcpy(auxmes, token);
        mes2 = obterMes(auxmes);

        token = strtok(NULL, " :");
        dia2 = atoi(token);

        token = strtok(NULL, " :");
        horas2 = atoi(token);

        token = strtok(NULL, " :");
        minutos2 = atoi(token);
 
        token = strtok(NULL, " :");
        seg2 = atoi(token);
 
        // guardando os dados do primeiro arquivo analisado
        if (i == 0) {
            mes1 = mes2;
            dia1 = dia2;
            horas1 = horas2;
            minutos1 = minutos2;
            seg1 = seg2;
        }
    }

    //momento de comparar
    ehRecente = 0;
    if (mes1 > mes2) 
        ehRecente = 1;
    else if (mes1 == mes2) {
        if (dia1 > dia2)
            ehRecente = 1;
        else if (dia1 == dia2) {
            if (horas1 > horas2)
                ehRecente = 1;
            else if (horas1 == horas2) {
                if (minutos1 > minutos2)
                    ehRecente = 1;
                else if (minutos1 == minutos2) {
                    if (seg1 > seg2)
                        ehRecente = 1;
                }
            }
        }
    }

    fclose(arq1);
    free(nome1);

    return ehRecente;
    
}

int obterMes (char* mes) {
    
    char* meses[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    int i;
    
    for (i = 0; i < 12; i++) {
        if (strcmp(mes, meses[i]) == 0) {
            return i + 1; // retorna o numero do mês
        }
    }

    return -1; // retorna -1 se o mês não for encontrado

}

void lerCabeca (FILE* archive, int* N, long int* posicaoDiretorio) {

    fseek(archive, 0, SEEK_SET);
    fread(N, sizeof(int), 1, archive);
    fread(posicaoDiretorio, sizeof(long int), 1, archive);

}

void guardarDiretorio (FILE* archive, struct lista* diretorio) {

    int ordemArq, blocoNome, blocoLocal;
    long int ultimaMod;
    uid_t UID;
    mode_t permissoes;
    off_t tam;

    // aloca um novo nodo e guarda os seus meta dados
    while (fread(&blocoNome, sizeof(int), 1, archive) == 1) {

        struct nodo* novo = malloc(sizeof(struct nodo));
            
        char auxNome[blocoNome];
        fread(&auxNome, sizeof(char), blocoNome, archive);
        strcpy(novo->nome, auxNome);

        fread(&UID, sizeof(uid_t), 1, archive);
        novo->UID = UID;

        fread(&permissoes, sizeof(mode_t), 1, archive);
        novo->permissoes = permissoes;
        
        fread(&tam, sizeof(off_t), 1, archive);
        novo->tam = tam;

        fread(&ultimaMod, sizeof(long int), 1, archive);
        novo->ultimaMod = ultimaMod;

        fread(&ordemArq, sizeof(int), 1, archive);
        novo->ordemArq = ordemArq;

        fread(&blocoLocal, sizeof(int), 1, archive);
        char auxLocal[blocoLocal];
        fread(&auxLocal, sizeof(char), blocoLocal, archive);
        strcpy(novo->local, auxLocal);

        inserirNodo(diretorio, novo);

    }
}

void atualizarDiretorio (struct lista* diretorio, FILE* arq, char* nome, int N) {

    struct nodo* novo = malloc(sizeof(struct nodo));

    // cria um novo nodo e guarda seus meta dados na struct nodo
    novo = criarNodo(novo, arq, nome);
   
    // insere o nodo na lista
    inserirNodo(diretorio, novo);

    // gaurda a ordem de inserção do nodo em questão
    novo->ordemArq = diretorio->tam;
    
}

struct nodo* buscar (struct lista* diretorio, char* nome) {

    struct nodo* aux;
    char* ptrNome;
    int i, temHierarquia;
    char bufferLocal[TAM_BUFFER];

    /* trata as diferentes mandeiras 
    de como o nome do arquivo foi passado*/
    temHierarquia = 0;
    ptrNome = NULL;
    ptrNome = strrchr(nome, '/');
    if (ptrNome != NULL)
        temHierarquia = 1;     // identifica se nome do arquivo foi passado com caminho

    if (temHierarquia == 0) {
        strcpy(bufferLocal, "./");
        strcat(bufferLocal, nome);
    }
    else if(nome[0] == '/') {
        strcpy(bufferLocal, ".");
        strcat(bufferLocal, nome);
    }
    else if(nome[0] != '.') {
        strcpy(bufferLocal, "./");
        strcat(bufferLocal, nome);
    }
    else if (nome[0] == '.' && nome[0] == '.')
        strcpy(bufferLocal, nome);
    else
        strcpy(bufferLocal, nome);

    i = 0;
    for (aux = diretorio->inicio; i < diretorio->tam; aux = aux->proximo) {
        if (strcmp(aux->local, bufferLocal) == 0)
            return aux;
        i++;
    }

    return NULL;
}

void atualizarCabeca (FILE* archive, struct lista* diretorio) {

    fseek(archive, 0, SEEK_SET);
    fwrite(&diretorio->tam, sizeof(int), 1, archive);
    fwrite(&diretorio->posicao, sizeof(long int), 1, archive);
    
}

void escreverDiretorio (FILE* archive, struct lista* diretorio) {

    struct nodo* aux;
    int i, blocoNome, blocoLocal;    

    i = 0;

    // varre o diretorio escrevendo os meta dados dos nodos no archiver
    for (aux = diretorio->inicio; i < diretorio->tam; aux = aux->proximo) {

        blocoNome = strlen(aux->nome)+1;
        blocoLocal = strlen(aux->local)+1;

        fwrite(&blocoNome, sizeof(int), 1, archive);
        fwrite(&aux->nome, sizeof(char), blocoNome, archive);
        fwrite(&aux->UID, sizeof(uid_t), 1, archive);
        fwrite(&aux->permissoes, sizeof(mode_t), 1, archive);
        fwrite(&aux->tam, sizeof(off_t), 1, archive);
        fwrite(&aux->ultimaMod, sizeof(long int), 1, archive);
        fwrite(&aux->ordemArq, sizeof(int), 1, archive);
        fwrite(&blocoLocal, sizeof(int), 1, archive);
        fwrite(&aux->local, sizeof(char), blocoLocal, archive);
        i++;
    }    
}

void imprimir (char *argv[], struct lista* diretorio) {

    int i = 0;
    int N;
    long int posicaoDiretorio;
    struct nodo* aux;

    FILE* archive = fopen(argv[2], "r+");
    if (archive == NULL) {
        printf("Erro ao abrir %s.\n", argv[2]);
        exit (1);
    }

    if (ehVazio(archive) == 0) {
        printf("Archive está vazio! Não há o que imprimir.\n");
    }
    else {

        // lê o num de membros e a posição para o diretório
        lerCabeca(archive, &N, &posicaoDiretorio);
        diretorio->posicao = posicaoDiretorio;

        // desloca para a posição do diretório para poder guardá-lo na lista encadeada
        fseek(archive, diretorio->posicao, SEEK_SET);
        guardarDiretorio(archive, diretorio);

        printf("  Permissoes  |  UID  |  bytes  |  ordem  |  data de modificacao  |     nome\n");
        for (aux = diretorio->inicio; i < diretorio->tam; aux = aux->proximo){
            
            // Permissões do proprietário
            printf("  ");
            printf((aux->permissoes & S_IRUSR) ? "r" : "-");
            printf((aux->permissoes & S_IWUSR) ? "w" : "-");
            printf((aux->permissoes & S_IXUSR) ? "x" : "-");

            // Permissões do grupo
            printf((aux->permissoes & S_IRGRP) ? "r" : "-");
            printf((aux->permissoes & S_IWGRP) ? "w" : "-");
            printf((aux->permissoes & S_IXGRP) ? "x" : "-");

            // Permissões de outros
            printf((aux->permissoes & S_IROTH) ? "r" : "-");
            printf((aux->permissoes & S_IWOTH) ? "w" : "-");
            printf((aux->permissoes & S_IXOTH) ? "x" : "-");

            struct tm *timeInfo;
            char buffer[80];
            timeInfo = localtime(&aux->ultimaMod);
            strftime(buffer, sizeof(buffer), "%b %e %H:%M", timeInfo);

            printf("%9d %9ld %7i          %s         %s", aux->UID, aux->tam, aux->ordemArq, buffer, aux->nome);

            printf("\n");
            i++;
        }
    }

    fclose(archive);
}

void mover (char* argv[], struct lista* diretorio) {

    int i, N;
    long int posicaoDiretorio;
    char* nomeAlvo = NULL;
    char* nomeTarget = NULL;
    struct nodo* target, *alvo, *anterior, *aux;
    

    FILE* archive = fopen(argv[3], "r+");
    if (!archive) {
        printf("[função mover]\n");
        printf("Erro: não foi possível abrir o arquivo %s.\n", argv[4]);
        exit(1);
    }

    // lê o num de membros e a posição do diretorio 
    lerCabeca(archive, &N, &posicaoDiretorio);

    // desloca para a posição do diretório para guardá-lo na lista diretorio
    fseek(archive, posicaoDiretorio, SEEK_SET);
    guardarDiretorio(archive, diretorio);

    if (diretorio->tam == 1) {
        printf("O %s possui apenas um membro, logo não é possível fazer movimentação.\n", argv[3]);
    }
    else {

        // target corresponde ao arquivo usado como 'target' na função de mover
        nomeTarget = strdup(argv[2]);
        target = buscar(diretorio, nomeTarget);

        if (target == NULL) {
            printf("O arquivo %s não está em %s, logo é impossível usá-lo como target (-m target).\n", argv[2], argv[3]);
        }
        else {

            // alvo corresponde ao arquivo que será deslocado pela função mover
            nomeAlvo = strdup(argv[4]);
            alvo = buscar(diretorio, nomeAlvo);

            if (alvo == NULL) {
                printf("O arquivo %s não está em %s, logo é impossível movê-lo.\n", argv[4], argv[3]);
            }
            else {

                // verificando que target e alvo não foram passados iguais pelo usuário
                if (target != alvo) { 

                    // obtendo o nodo anterior do nodo alvo
                    anterior = NULL;
                    for (aux = diretorio->inicio; aux != alvo; aux = aux->proximo) {
                        anterior = aux;
                        i++;
                    }

                    if (diretorio->inicio == alvo) {        // alvo está no início da lista
                        diretorio->inicio = alvo->proximo;
                        alvo->proximo = target->proximo;
                        target->proximo = alvo;
                    }
                    else if (diretorio->fim == alvo) {      // alvo está no final da lista
                        anterior->proximo = alvo->proximo;
                        diretorio->fim = anterior;
                        alvo->proximo = target->proximo;
                        target->proximo = alvo;
                    }
                    else {
                        anterior->proximo = alvo->proximo;
                        alvo->proximo = target->proximo;
                        target->proximo = alvo;
                    }
                }
                // descartando o diretorio
                posicaoDiretorio = totalBytes(diretorio);
                truncate(argv[3], posicaoDiretorio);

                // escreve o conteúdo do diretorio no archive
                fseek(archive, posicaoDiretorio, SEEK_SET);
                escreverDiretorio(archive, diretorio);
                fflush(archive);
            }
        }        
    }
    
    free(nomeTarget);
    free(nomeAlvo);
    fclose(archive);
}

void ajustarOrdem (struct lista* diretorio, int ordemExcluida) {

    // ajusta a ordem de inserção dos membros,
    // as alterações de ordem serão feitas nos membros que tinham ordem superior
    // em relação a ordemEcluída (que é a ordem do arquivo excluído)

    int i;
    struct nodo* aux;

    i = 0;
    for(aux = diretorio->inicio; i < diretorio->tam; aux = aux->proximo) {
        if (aux->ordemArq > ordemExcluida)
            aux->ordemArq--;
        i++;
    }
}

long int calcularTruncate (struct lista* diretorio, int ordemArq) {

    // calcula o tamanho do archiver descartando os bytes
    // do Membro excluído, sendo ordemArq a ordem de inserção do Membro em questão

    int i;
    long int cont;
    struct nodo* aux;

    cont = sizeof(int) + sizeof(long int);

    i = 0;
    for (aux = diretorio->inicio; i < diretorio->tam; aux = aux->proximo) {
        if(aux->ordemArq != ordemArq)
            cont += aux->tam;
        i++;
    }
    
    return cont;
}

off_t totalBytes (struct lista* diretorio) {

    // calcula o tamanho total do archiver, sem levar em consideração o tam do diretório
    // ou seja, contabiliza a cabeça e os Membros

    int i;
    off_t cont;
    struct nodo* aux;

    i = 0;
    cont = sizeof(int) + sizeof(long int);

    for (aux = diretorio->inicio; i < diretorio->tam; aux = aux->proximo) {
        cont += aux->tam;
        i++;
    }
    
    return cont;
}

long int obterPosicao (FILE* archive, struct lista* diretorio, int ordemArq) {

    // calcula o ponteiro para a posição de inicio do conteúdo do Membro,
    // sendo ordemArq a ordem de inserção do Membro em questão

    int i;
    long int cont;
    struct nodo* aux;

    i = 0;
    cont = sizeof(int) + sizeof(long int);
    
    for (aux = diretorio->inicio; i < diretorio->tam; aux = aux->proximo) {
        if (aux->ordemArq < ordemArq)
            cont += aux->tam;
        i++;
    }

    return cont;
}