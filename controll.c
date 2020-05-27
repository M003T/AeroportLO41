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

#include "controll.h"
#include "semaphore.h"

void controll ()
{
	while (exitrequested!= 1)
	{
		//Création message
		FlightInformationMsg.type = 1;
		FlightInformationMsg.tracknumber = (rand()%2)+1;
		FlightInformationMsg.routebrief = "Tout droit";
		FlightInformationMsg.trackbrief = "A gauche";
		FlightInformationMsg.liftoffhour = time(NULL);
		FlightInformationMsg.maxdelay = rand()%30;

		V(WaitFlightInformation);

		if (msgsnd(msgid, &FlightInformationMsg, sizeof(FlightInformation) - sizeof(long),0) == -1) 
		{
	    	perror("Erreur d'écriture msg\n");
	    	exit(1);
		}
		
	}
}