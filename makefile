CC = gcc
CFlags = -Wall -c
LDFLAGS =
EXEC = main

all: $(main)

main : main.o controle.o
	$(CC) $^ -o $a $(LDFLAGS)

main.o : main.c
	$(CC) $< -o $@ $(CFlags)

controle.o : controle.c controle.h
	$(CC) $< -o $@ $(CFlags)