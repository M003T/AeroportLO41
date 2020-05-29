#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "sharedmemory.h"
#include "ipc.h"

int shm_id;

void initshm ()
{
	if ( (shm_id = shmget(KEY,sizeof(SharedMemoryStruct), IFLAGS) ) < 0 )
	{
		perror("Erreur initialisation mémoire partagée");
		//exit(EXIT_FAILURE);
    }
}

void* getshm (void* adr)
{
	return shmat(shm_id,adr,0);
}

void removeshm (void* adr)
{
	if ( shmdt(adr) < 0 )
	{
		perror("Erreur détachement mémoire partagée");
		//exit(EXIT_FAILURE);
    }
}

void deleteshm ()
{
	if ( shmctl(shm_id,IPC_RMID, NULL) )
	{
		perror("Erreur initialisation mémoire partagée");
		//exit(EXIT_FAILURE);
    }
}