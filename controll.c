#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/msg.h>
#include <stdlib.h>

#include "sharedmemory.h"
#include "messagefile.h"
#include "controll.h"
#include "semaphore.h"

PlaneInformationStruct PlaneInformation;
FlightInformationStruct FlightInformation;

void controll ()
{
	//Sémaphore Mutex d'accès à NbPlaneAwaitingInformation dans la boucle while
	P(MutexNbPlaneAwaitingInformation);
	while ((SharedMemory->exitrequested == 0) || (SharedMemory->NbPlaneAwaitingInformation > 0))
	{	
		V(MutexNbPlaneAwaitingInformation);

		P(MutexNbPlaneAwaitingInformation);
		if(SharedMemory->NbPlaneAwaitingInformation > 0)
		{
			V(MutexNbPlaneAwaitingInformation);
			V(WaitFlightInformation);
			receivePlaneInformation();
			printPlaneInformation();
			generateFlightInformation();
			printFlightInformation();
			sendFlightInformation();
		}
		else
			V(MutexNbPlaneAwaitingInformation);

		P(MutexNbPlaneAwaitingInformation);
	}
	V(MutexNbPlaneAwaitingInformation);
}

void receivePlaneInformation ()
{
	errno = 0;
	if ((msgrcv(msg_id, &PlaneInformation, sizeof(PlaneInformationStruct) - sizeof(long), 1, 0)) == -1) 
	{
		//En cas de Ctrl-C tout message envoyé est interrompu, on le relance afin que le programme se ferme correctement
		if (errno == EINTR)
	    	receivePlaneInformation();
	    else
	    {
	    	perror("Erreur de lecture message avion");
	    	//exit(EXIT_FAILURE);
	    }
	}
}

void printPlaneInformation()
{
	if (PlaneInformation.size == 2)
		printf("Avion n°%d (Gros Porteur) ",PlaneInformation.num);
	else
		printf("Avion n°%d (Petit Calibre) ",PlaneInformation.num);
	if (PlaneInformation.fromorto == 1)
		printf("en provenance de %s",PlaneInformation.destination);
	else
		printf("à destination de %s",PlaneInformation.destination);
	printf(" est en attente d'informations\n");
}

void generateFlightInformation ()
{
	FlightInformation.type = 2;

	if (PlaneInformation.size == 1)
		FlightInformation.tracknumber = (rand()%2)+1; // A changer ??
	else
		FlightInformation.tracknumber = 2;

	if (FlightInformation.tracknumber == 1)
	{
		strcpy(FlightInformation.routebrief,"Tout droit vers la grande piste");
		strcpy(FlightInformation.trackbrief,"Sortir vers les pistes");
	}
	else
	{
		strcpy(FlightInformation.routebrief,"A droite vers la petite piste");
		strcpy(FlightInformation.trackbrief,"Sortir vers les pistes");
	}
	time_t mytime;
	mytime = time(NULL);
	FlightInformation.liftoffhour = *localtime( &mytime );
	FlightInformation.maxdelay = rand()%30;
}

void sendFlightInformation ()
{	
	errno = 0;
	if (msgsnd(msg_id, &FlightInformation, sizeof(FlightInformationStruct) - sizeof(long),0) == -1) 
	{
		//En cas de Ctrl-C tout message envoyé est interrompu, on le relance afin que le programme se ferme correctement
	    if (errno == EINTR)
	    	sendFlightInformation();
	    else
	    {
	    	perror("Erreur d'envoi message tour de contrôle");
	    	//exit(EXIT_FAILURE);
	    }
	}
}

void printFlightInformation()
{
	printf("Avion n°%d : ",PlaneInformation.num);
	if (PlaneInformation.fromorto == 1)
		printf("arrivée piste %d",FlightInformation.tracknumber);
	else
		printf("départ piste %d",FlightInformation.tracknumber);
	printf(" à %d:%d, délai max %d min\n",FlightInformation.liftoffhour.tm_hour,FlightInformation.liftoffhour.tm_min,FlightInformation.maxdelay);
}