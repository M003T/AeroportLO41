#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#define IFLAGS (SEMPERM | IPC_CREAT)
#define SKEY   (key_t) IPC_PRIVATE
#define SEMPERM 0600

#define WaitFlightInformation 0
#define MutexNbPlaneAwaitingInformation 1

struct sembuf sem_oper_P ;
struct sembuf sem_oper_V ;

extern int sem_id;

void P(int );

void V(int );

int initsem(key_t );

int liberesem();

#endif