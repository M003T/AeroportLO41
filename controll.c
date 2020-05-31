#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>

#include "sharedmemory.h"
#include "messagefile.h"
#include "controll.h"
#include "semaphore.h"

PlaneInformationStruct PlaneInformation;
FlightInformationStruct FlightInformation;

void controll ()
{
	//Sémaphores Mutex d'accès aux variables monitrice afin que le programme ne s'arrête pas tant que des avions sont encore présent
	P(MutexNbPlaneAwaitingInformation);
	P(MutexTrack1);
	P(MutexTrack2);
	while ((SharedMemory->exitrequested == 0) || (SharedMemory->NbPlaneAwaitingInformation > 0) || (SharedMemory->NbPlaneAwaitingTrack1 > 0) || (SharedMemory->NbPlaneAwaitingTrack2 > 0) || (SharedMemory->Track1Used == 1) || (SharedMemory->Track2Used == 1))
	{	
		V(MutexNbPlaneAwaitingInformation);
		V(MutexTrack1);
		V(MutexTrack2);

		//Si avions attendent information alors on leur envoie
		testPlaneAwaitingInformation();

		//Si avions attendent sur la piste 1 alors on les déverouille
		testTrack1();
		
		//Si avions attendent sur la piste 2 alors on les déverouille
		testTrack2();
		
		//Réactivation de ces mêmes Mutex à la fin de la boucle while afin que leur consultation pour l'itération suivante du while soit protégée
		P(MutexNbPlaneAwaitingInformation);
		P(MutexTrack1);
		P(MutexTrack2);
	}
	V(MutexNbPlaneAwaitingInformation);
	V(MutexTrack1);
	V(MutexTrack2);
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
	{
		P(MutexTrack1);
		P(MutexTrack2);
		//Si la petite piste est en surcharge par rapport à la grande, les avions petit calibre décollent sur la grande
		if(SharedMemory->NbPlaneAwaitingTrack1 <= (SharedMemory->NbPlaneAwaitingTrack2 - 3))
			FlightInformation.tracknumber = 1;
		else
			FlightInformation.tracknumber = 2;
		V(MutexTrack1);
		V(MutexTrack2);
	}
	else
		FlightInformation.tracknumber = 1;

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
	//Création de l'heure de départ : l'heure actuelle + de 0 à 2 min ajoutées, si le champ des minutes dépasse 60 alors on augmente d'1h et on enlève 60 min
	time_t mytime = time(NULL);
	FlightInformation.takeofforlandinghour = *localtime( &mytime );
	int minutestoadd = (rand()%3);
	if ( (FlightInformation.takeofforlandinghour.tm_min + minutestoadd) >=60 )
	{
		FlightInformation.takeofforlandinghour.tm_hour++;
		FlightInformation.takeofforlandinghour.tm_min = FlightInformation.takeofforlandinghour.tm_min + minutestoadd - 60;
	}
	else
	{
		FlightInformation.takeofforlandinghour.tm_min = FlightInformation.takeofforlandinghour.tm_min + minutestoadd;
	}
	FlightInformation.maxdelay = (rand()%3)+1;
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
		printf("arrivée prévue piste %d",FlightInformation.tracknumber);
	else
		printf("départ prévu piste %d",FlightInformation.tracknumber);
	printf(" à %d:%d, délai max %d min\n",FlightInformation.takeofforlandinghour.tm_hour,FlightInformation.takeofforlandinghour.tm_min,FlightInformation.maxdelay);
}

void testPlaneAwaitingInformation()
{
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
}

void testTrack1()
{
	P(MutexTrack1);
	if(SharedMemory->NbPlaneAwaitingTrack1 > 0 && SharedMemory->Track1Used == 0 )
	{
		V(MutexTrack1);
		V(WaitTrack1);
		//Sleep nécessaire pour éviter que la tour de contrôle ait le temps de faire un tour de boucle complet et déverouiller un 2ème avion avant que ce dernier ait le temps de décoller
		sleep(1);
	}
	else
		V(MutexTrack1);
}

void testTrack2()
{
	P(MutexTrack2);
	if(SharedMemory->NbPlaneAwaitingTrack2 > 0 && SharedMemory->Track2Used == 0 )
	{
		V(MutexTrack2);
		V(WaitTrack2);
		sleep(1);
	}
	else
		V(MutexTrack2);
}