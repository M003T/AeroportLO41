CC = gcc
CFlags = -Wall -c -pthread
LDFLAGS = -pthread
EXEC = main

all: $(EXEC)

main : main.o controle.o
	$(CC) $^ -o $@ $(LDFLAGS)

main.o : main.c
	$(CC) $< -o $@ $(CFlags)

controle.o : controle.c controle.h
	$(CC) $< -o $@ $(CFlags)