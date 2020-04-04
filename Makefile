#Makefile Variables
CC = clang
CFLAGS = -Wall -Werror -std=c11 -g -pthread -fsanitize=thread 
LDFLAGS= -pthread -fsanitize=thread
OBJ = main.o linkedlist.o
EXEC = assignment

$(EXEC) : $(OBJ)
		$(CC) $(OBJ) -o $(EXEC) -g $(LDFLAGS)

main.o : main.c structs.h
		$(CC) main.c -c $(CFLAGS)

linkedlist.o : linkedlist.c linkedlist.h
		$(CC) linkedlist.c -c $(CFLAGS)

clean :
		rm -f $(OBJ) $(EXEC)
