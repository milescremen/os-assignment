#Makefile Variables
CC = clang
CFLAGS=-Wall -std=c99 -Wextra -pedantic -g
CFLAGS+=-Wformat=2 -Wswitch-default -Wswitch-enum
CFLAGS+=-Wpointer-arith -Wbad-function-cast
CFLAGS+=-Wstrict-overflow=5 -Wstrict-prototypes
CFLAGS+=-Winline -Wundef -Wnested-externs
CFLAGS+=-Wcast-qual -Wshadow -Wunreachable-code
CFLAGS+=-Wfloat-equal -Wstrict-aliasing
CFLAGS+=-Wredundant-decls -Wold-style-definition
CFLAGS+=-ggdb3 -O0 -fno-omit-frame-pointer
CFLAGS+=-fno-common -Wdouble-promotion -Wcast-align
CFLAGS+=-Winit-self
CFLAGS+=-fsanitize=unsigned-integer-overflow,nullability,float-divide-by-zero

OBJ = main.o linkedlist.o
EXEC = assignment

$(EXEC) : $(OBJ)
		$(CC) $(OBJ) -o $(EXEC) -g

main.o : main.c structs.h
		$(CC) main.c -c $(CFLAGS)

linkedlist.o : linkedlist.c linkedlist.h
		$(CC) linkedlist.c -c $(CFLAGS)

clean :
		rm -f $(OBJ) $(EXEC)
