CC = gcc
CFlags = -Wall -c -pthread -g
LDFLAGS = -pthread
EXEC = main

all: $(EXEC)

main : main.o controll.o plane.o semaphore.o sharedmemory.o messagefile.o
	$(CC) $^ -o $@ $(LDFLAGS)

main.o : main.c controll.h plane.h semaphore.h messagefile.h sharedmemory.h
	$(CC) $< -o $@ $(CFlags)

controll.o : controll.c controll.h semaphore.h sharedmemory.h
	$(CC) $< -o $@ $(CFlags)

plane.o : plane.c plane.h semaphore.h
	$(CC) $< -o $@ $(CFlags)

semaphore.o : semaphore.c semaphore.h ipc.h
	$(CC) $< -o $@ $(CFlags)

sharedmemory.o : sharedmemory.c sharedmemory.h ipc.h
	$(CC) $< -o $@ $(CFlags)

messagefile.o : messagefile.c messagefile.h ipc.h
	$(CC) $< -o $@ $(CFlags)

debug :
	gdb -ex=r --args main 10 