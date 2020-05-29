#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "messagefile.h"
#include "ipc.h"

int msg_id;

void initmsgfile ()
{
	if ( (msg_id = msgget(KEY, IFLAGS) ) < 0) 
	{
		perror("Erreur de creation file de message");
		//exit(EXIT_FAILURE);
    }
}

void deletemsgfile ()
{
	if ( msgctl(msg_id, IPC_RMID, NULL) < 0 )
	{
		perror("Erreur fermeture file de messages");
		//exit(EXIT_FAILURE);
    }
}