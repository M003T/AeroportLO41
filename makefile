CC = gcc
CFlags = -Wall -c -pthread -g
LDFLAGS = -pthread
EXEC = main

all: $(EXEC)

main : main.o controll.o semaphore.o plane.o
	$(CC) $^ -o $@ $(LDFLAGS)

main.o : main.c controll.h msgstructs.h  plane.h semaphore.h
	$(CC) $< -o $@ $(CFlags)

semaphore.o : semaphore.c semaphore.h
	$(CC) $< -o $@ $(CFlags)

controll.o : controll.c controll.h semaphore.h
	$(CC) $< -o $@ $(CFlags)

plane.o : plane.c plane.h
	$(CC) $< -o $@ $(CFlags)

debug:
	gdb -ex=r --args main 10 