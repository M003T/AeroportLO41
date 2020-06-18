#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <sys/sem.h>

#define WaitFlightInformation 0
#define WaitTrack1 1
#define WaitTrack2 2
#define MutexNbPlaneAwaitingInformation 3
#define MutexTrack1 4
#define MutexTrack2 5
#define MutexPlanesWaiting 6
#define MutexBarrier1 7
#define MutexBarrier2 8
#define MutexPlaneSignal 9

struct sembuf sem_oper_P ;
struct sembuf sem_oper_V ;

extern int sem_id;

void P(int );

void V(int );

void initsem();

void deletesem();

#endif