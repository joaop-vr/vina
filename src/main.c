#include <stdio.h>
#include <stdlib.h>

#include "complementares.h"
#include "lista.h"

int main(int argc, char *argv[]) {

    /* Iniciando a lista dos arquivos */
    struct lista* diretorio = gerarDiretorio();

    operacoes(argc, argv, diretorio);

    destruirLista(diretorio);
    
    return 0;
}