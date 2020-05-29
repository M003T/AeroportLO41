#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <sys/sem.h>

#define WaitFlightInformation 0
#define MutexNbPlaneAwaitingInformation 1

struct sembuf sem_oper_P ;
struct sembuf sem_oper_V ;

extern int sem_id;

void P(int );

void V(int );

void initsem();

void deletesem();

#endif