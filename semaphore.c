#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/msg.h>

#include "semaphore.h"

int sem_id ;

void P(int semnum) {

sem_oper_P.sem_num = semnum;
sem_oper_P.sem_op  = -1 ;
sem_oper_P.sem_flg = 0 ;
semop(sem_id,&sem_oper_P,1);
}

void V(int semnum) {

sem_oper_V.sem_num = semnum;
sem_oper_V.sem_op  = 1 ;
sem_oper_V.sem_flg  = 0 ;
semop(sem_id,&sem_oper_V,1);
}

int initsem(key_t semkey) 
{
    
	int status = 0;		
	int semid_init;
   	union semun {
		int val;
		struct semid_ds *stat;
		short * array;
	} ctl_arg;
    if ((semid_init = semget(semkey, 2, IFLAGS)) > 0) {
		
	    	short array[2] = {0,1};
	    	ctl_arg.array = array;
	    	status = semctl(semid_init, 0, SETALL, ctl_arg);
    }
   if (semid_init == -1 || status == -1) { 
	perror("Erreur initsem");
	return (-1);
    } else return (semid_init);
}

int liberesem()
{
	return semctl(sem_id, 0, IPC_RMID,0);
}