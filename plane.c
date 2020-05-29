#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdint.h>
#include <pthread.h>

#include "semaphore.h"
#include "sharedmemory.h"
#include "messagefile.h"
#include "plane.h"


FlightInformationStruct FlightInformation;
PlaneInformationStruct PlaneInformation;

void * plane (void * arg)
{
	//Recuperation de son numero et initialisation informations initiales
	PlaneInformation.num = ((int) (intptr_t) arg) + 714;
	initPlaneInformation();

	//Se met dans la file d'attente des avions en attente d'information
	P(MutexNbPlaneAwaitingInformation);
	SharedMemory->NbPlaneAwaitingInformation++;
	V(MutexNbPlaneAwaitingInformation);
	
	P(WaitFlightInformation);

	//Se retire de cette liste
	P(MutexNbPlaneAwaitingInformation);
	SharedMemory->NbPlaneAwaitingInformation--;
	V(MutexNbPlaneAwaitingInformation);

	//Envoi message informations avion initiales
	sendPlaneInformation();

	//Reception Message Informations de vol
	receiveFlightInformation();
	
	pthread_exit(NULL);
}

void initPlaneInformation()
{
	PlaneInformation.type = 1;

	//Choix aléatoire destination
	int destinationnumber = rand() % 30;
	if (destinationnumber <= 19)
		strcpy(PlaneInformation.destination,FranceDestinations[destinationnumber]);
	else
		strcpy(PlaneInformation.destination,EuropeDestinations[destinationnumber-20]);

	if (rand()%4 != 0)
		PlaneInformation.fromorto = 0;
	else
		PlaneInformation.fromorto = 1;

	PlaneInformation.size = (rand()%2)+1;
}

void sendPlaneInformation()
{
	errno = 0;
	if (msgsnd(msg_id, &PlaneInformation, sizeof(PlaneInformationStruct) - sizeof(long),0) == -1) 
		{
			//En cas de Ctrl-C tout message envoyé est interrompu, on le relance afin que le programme se ferme correctement
	    	if (errno == EINTR)
	    		sendPlaneInformation();
	    	else
	    	{
	    	perror("Erreur d'envoi message avion");
	    	//exit(EXIT_FAILURE);
	    	}
		}
}

void receiveFlightInformation()
{
	errno = 0;
	if ((msgrcv(msg_id, &FlightInformation, sizeof(FlightInformationStruct) - sizeof(long), 2, 0)) == -1) 
	{
		//En cas de Ctrl-C tout message envoyé est interrompu, on le relance afin que le programme se ferme correctement
	    if (errno == EINTR)
	    	receiveFlightInformation();
	    else
	    {
	    	perror("Erreur de lecture message tour de contrôle");
	    	//exit(EXIT_FAILURE);
	    }
	}
}
