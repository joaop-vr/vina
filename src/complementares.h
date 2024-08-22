#ifndef COMPLEMENTARES_H_
#define COMPLEMENTARES_H_
#include "lista.h"

/* ! funções de operações ! */

// imprimi o conteúdo do archive
void imprimir (char* argv[], struct lista* diretorio);

// desloca a sequência dos membros do archive
void mover (char* argv[], struct lista* diretorio);

/* ! funções de complementares ! */

// aloca memória para a lista encadeada
struct lista* gerarDiretorio();

// interpreta o comando passado pelo usuário e chama a função correspondente
void operacoes (int argc, char *argv[], struct lista* diretorio);

// retorna o tamanho de bytes do arqchive, se retornar 0 significa que está vazio
long int ehVazio (FILE* arq);

// adiciona à lista encadeada do diretório o novo membro adicionado
void atualizarDiretorio (struct lista* diretorio, FILE* arq, char* nome, int N);

// utilizada na operação "-a"
// checa se o membro é mais recente ou não em relação à versão arquivada
int checarRecente (char* argvAlvo, struct nodo* arquivado);

// utilizada na operação "-a"
// recebe o nome de um mês e retorna o número correspondente [ex.: recebe "Março" e retorna 3]
int obterMes (char* mes);

// retorna ptr para o nodo se foi encontrado, caso contrário retorna NULL
struct nodo* buscar (struct lista* diretorio, char* nome);

// lê cabeça no archive (número de membros e posição do diretório)
void lerCabeca (FILE* archive, int* N, long int* posicaoDiretorio);

// atualiza a cabeça do archiver com o seu novo número de membros e a nova posição do diretório
void atualizarCabeca (FILE* archive, struct lista* diretorio);

// escreve o conteúdo do diretório no archiver
void escreverDiretorio (FILE* archive, struct lista* diretorio);

// lê o diretório escrito no archiver e guarda esse conteúdo em uma lista encadeada
void guardarDiretorio (FILE* archive, struct lista* diretorio);

// ajusta a ordem de inserção dos membros,
// as alterações de ordem serão feitas nos membros que tinham ordem superior
// em relação a ordemEcluída (que consiste na ordem do arquivo excluído)
void ajustarOrdem (struct lista* diretorio, int ordemExcluida);

//Retorna a quantidade total de bytes que o archive possui - exceto pela qntd do diretorio
off_t totalBytes (struct lista* diretorio);

// retorna a posicao do membro cuja ordem no archive
//  foi passada por parametro "ordemArq"
//  será utilizada para realizar ftell
long int obterPosicao (FILE* archive, struct lista* diretorio, int ordemArq);

// calcula o tamanho do archiver descartando os bytes
// do Membro excluído, sendo ordemArq a ordem de inserção do Membro em questão
long int calcularTruncate (struct lista* diretorio, int ordemArq);

#endif