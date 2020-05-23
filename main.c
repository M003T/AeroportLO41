#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <sys/sem.h>
#include <errno.h>

#define IFLAGS (SEMPERM | IPC_CREAT)
#define SKEY   (key_t) IPC_PRIVATE	
#define SEMPERM 0600

int exitrequested;
int sem_id ;
struct sembuf sem_oper_P ;
struct sembuf sem_oper_V ;

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
    if ((semid_init = semget(semkey, 1, IFLAGS)) > 0) {
		
	    	short array[1] = {0};
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

void traitantSIGINT(int num) {

	if (num!=SIGINT)
    perror("Erreur SigInt\n");

	exitrequested = 1;
	}

void attenterandom(int n) {
   sleep(rand() % n);
}

void controll ()
{
	while (exitrequested!= 0)
	{

	}
}

void * plane ()
{
	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
	srand(time(0));

	if (argc-1 != 1) {
        perror("Veuillez passer en argument le nombre total d'avions pour cette simulations\n");
        return 1;
        }

	//Réecriture signal Ctrl-C pour que les objets IPC soient correctement supprimés en cas d'arrêt prématuré
	exitrequested = 0;
	signal(SIGINT,traitantSIGINT);

	//Initialisation sémaphores
    if ((sem_id = initsem(SKEY)) < 0)
		return(1);

	pid_t pid;
	pid = fork();

	if (pid == 0)
	{
		controll();
		exit(0);
	}
	else
	{
		int NBPlaneMax=atoi(argv[1]);
		int i=0;
		pthread_t thr[NBPlaneMax];
    	while (i<NBPlaneMax && exitrequested != 0)
		{
			if (pthread_create(&thr[i], NULL, plane, (void *) (intptr_t) i) != 0)
			{
				perror("Erreur Creation Thread\n");
				return(1);
			}
    		attenterandom(5);
    		i++;
		}
		int status;
		waitpid(pid,&status,0);
		pthread_join(thr[i-1],NULL);
		if (liberesem() < 0)
   			return(1);
	}

	
	return(0);
}