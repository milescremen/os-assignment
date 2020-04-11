#Makefile Variables
CC = clang
CFLAGS = -Wall -Werror -std=c11 -g -pthread -fsanitize=thread
LDFLAGS= -pthread -fsanitize=thread
OBJ = lifts.o queue.o
EXEC = assignment

$(EXEC) : $(OBJ)
		$(CC) $(OBJ) -o $(EXEC) -g $(LDFLAGS)

lifts.o : lifts.c lifts.h
		$(CC) lifts.c -c $(CFLAGS)

queue.o : queue.c queue.h
		$(CC) queue.c -c $(CFLAGS)

clean :
		rm -f $(OBJ) $(EXEC)

