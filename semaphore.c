#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdlib.h>

#include "semaphore.h"
#include "ipc.h"

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

void initsem() 
{
    
	int status = 0;
   	union semun {
		int val;
		struct semid_ds *stat;
		short * array;
	} ctl_arg;
    if ((sem_id = semget(KEY, 9, IFLAGS)) > 0) {
	    	short array[9] = {0,0,0,1,1,1,1,1,1};
	    	ctl_arg.array = array;
	    	status = semctl(sem_id, 0, SETALL, ctl_arg);
    }
	if (sem_id == -1 || status == -1) {
		perror("Erreur initsem");
		//exit(EXIT_FAILURE);
    }
}

void deletesem()
{
	if (semctl(sem_id, 0, IPC_RMID,0) < 0)
	{
		perror("Erreur fermeture sémaphore");
		//exit(EXIT_FAILURE);
	}
}