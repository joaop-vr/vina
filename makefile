CC=gcc
CFLAG= -std=c99 -D_XOPEN_SOURCE=700 -Wall 
LFLAG=
NAME=vina++

all: $(NAME)

$(NAME): src/main.o src/lista.o src/inserir.o src/excluir.o src/extrair.o src/complementares.o
	$(CC) $(CFLAG) src/main.o src/lista.o src/inserir.o src/excluir.o src/extrair.o src/complementares.o -o $(NAME)

src/main.o: src/main.c
	$(CC) -c $(CFLAG) src/main.c -o src/main.o

src/lista.o: src/lista.c
	$(CC) -c $(CFLAG) src/lista.c -o src/lista.o

src/inserir.o: src/inserir.c
	$(CC) -c $(CFLAG) src/inserir.c -o src/inserir.o

src/excluir.o: src/excluir.c
	$(CC) -c $(CFLAG) src/excluir.c -o src/excluir.o

src/extrair.o: src/extrair.c
	$(CC) -c $(CFLAG) src/extrair.c -o src/extrair.o

src/complementares.o: src/complementares.c
	$(CC) -c $(CFLAG) src/complementares.c -o src/complementares.o

clean:
	rm -f src/*.o src/*.gch $(NAME)
