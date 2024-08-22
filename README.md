# Concatenador de Arquivos - Vina++

## Autor
João Pedro Vicente Ramalho

## Objetivo

Este projeto tem como objetivo implementar um concatenador de arquivos onde o início do arquivo concatenado corresponde a uma região com os metadados dos arquivos adicionados. Cada novo arquivo é alocado ao final do arquivo concatenador, chamado de "vina".

## Estrutura do Projeto

O projeto está organizado nos seguintes arquivos:

### `main.c`
Contém as chamadas para as funções `gerarDiretorio()` e `operacoes()` (implementadas em `complementares.c`) e para a função `destruirLista()` (implementada em `lista.c`).

### `lista.c`
Implementa funções responsáveis pela manipulação dos nodos da lista encadeada que representa o diretório:

- **`inserirNodo()`**: Insere um novo nodo na lista.
- **`criarNodo()`**: Armazena os metadados do arquivo que está sendo adicionado em uma `struct nodo`.
- **`destruirLista()`**: Desaloca a lista encadeada.

**Estruturas:**

- `struct lista`: Armazena o número de membros, posição do diretório, e ponteiros para o início e fim da lista.
- `struct nodo`: Contém os metadados de cada arquivo, como nome, UID, permissões, tamanho, data de modificação, e ordem de inserção.

### `inserir.c`
Implementa funções para a inserção de arquivos no "vina":

- **`inserir()`**: Chama `iniciarArchive()` se o "vina" estiver vazio, ou `inserirMembro()` para adicionar novos arquivos.
- **`inserirMembro()`**: Insere os metadados do novo membro no "vina" e chama `escreverMembro()` para gravar o conteúdo.
- **`substituirMembro()`**: Substitui uma versão arquivada menos recente pela nova versão.

### `excluir.c`
Implementa funções para excluir arquivos do "vina":

- **`excluir()`**: Chama `excluirMembro()` para cada arquivo passado pelo usuário.
- **`excluirMembro()`**: Realiza um "shift" no conteúdo do "vina" para sobrescrever o arquivo excluído e reescreve o diretório.
- **`excluirNoDiretorio()`**: Remove o nodo correspondente ao arquivo excluído do diretório.

### `extrair.c`
Implementa funções para extrair arquivos do "vina":

- **`extrair()`**: Extrai arquivos específicos ou todos os membros do "vina".
- **`criarArquivo()`**: Recria a hierarquia de diretórios e o arquivo com permissões adequadas antes de chamar `transcreverMembro()`.
- **`transcreverMembro()`**: Transcreve o conteúdo do membro para o arquivo recém-criado.

### `complementares.c`
Implementa operações adicionais e funções de uso geral:

- **`imprimir()`**: Imprime os metadados de cada membro do diretório.
- **`mover()`**: Move um arquivo para logo depois de outro especificado, manipulando os ponteiros na lista encadeada.

## Decisões de Implementação

- **Arquivos Modulares**: Optei por criar arquivos individuais `.c` e `.h` para operações com muitas funções, como inserir, excluir e extrair. As operações de "mover" e "imprimir" foram agrupadas em `complementares.c` junto com funções de uso geral.

- **Estrutura de Dados**: Utilizei uma lista simplesmente encadeada para armazenar os metadados dos arquivos. Cada nodo da lista contém informações detalhadas sobre o arquivo, como nome, permissões, tamanho e caminho.

- **Manipulação Eficiente**: As operações de inserção e exclusão manipulam diretamente os ponteiros na lista, permitindo um gerenciamento eficiente dos arquivos dentro do "vina".

## Como Compilar e Executar

### Compilação

Para compilar o projeto, utilize o seguinte comando:

```bash
make
```

## Execução

Para inserir, excluir, extrair ou mover arquivos no "vina", utilize os seguintes comandos:

- **Inserir**: `./vina++ -i arquivo1 arquivo2 ...`
- **Excluir**: `./vina++ -d arquivo1 arquivo2 ...`
- **Extrair**: `./vina++ -x arquivo1 arquivo2 ...`
- **Mover**: `./vina++ -m target arquivo`
