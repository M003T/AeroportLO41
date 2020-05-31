#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#include "semaphore.h"
#include "sharedmemory.h"
#include "messagefile.h"
#include "plane.h"

//Mot clé __thread nécessaire pour créer une variable globale unique à chaque thread
__thread FlightInformationStruct LocalFlightInformation;
__thread PlaneInformationStruct LocalPlaneInformation;

void * plane (void * arg)
{
	//Recuperation de son numero et initialisation informations initiales
	LocalPlaneInformation.num = ((int) (intptr_t) arg) + 714;
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

	testtimetogo();

	takeOffOrLanding();
	
	pthread_exit(NULL);
}

void initPlaneInformation()
{
	LocalPlaneInformation.type = 1;

	//Choix aléatoire destination
	int destinationnumber = rand() % 30;
	if (destinationnumber <= 19)
		strcpy(LocalPlaneInformation.destination,FranceDestinations[destinationnumber]);
	else
		strcpy(LocalPlaneInformation.destination,EuropeDestinations[destinationnumber-20]);

	if (rand()%4 != 0)
		LocalPlaneInformation.fromorto = 0;
	else
		LocalPlaneInformation.fromorto = 1;

	LocalPlaneInformation.size = (rand()%2)+1;
}

void sendPlaneInformation()
{
	errno = 0;
	if (msgsnd(msg_id, &LocalPlaneInformation, sizeof(PlaneInformationStruct) - sizeof(long),0) == -1) 
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
	if ((msgrcv(msg_id, &LocalFlightInformation, sizeof(FlightInformationStruct) - sizeof(long), 2, 0)) == -1) 
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

void testtimetogo()
{
	time_t timestruct = time(NULL);
	struct tm actualtime = *localtime( &timestruct );
	//Tant que l'heure actuelle est différent de l'heure de décollage où que l'heure actuelle est inférieure à l'heure de décollage on attend
	while( (LocalFlightInformation.takeofforlandinghour.tm_hour != actualtime.tm_hour) || (actualtime.tm_min < LocalFlightInformation.takeofforlandinghour.tm_min) )
	{
		sleep(10);
		timestruct = time(NULL);
		actualtime = *localtime( &timestruct );
	}
}

void takeOffOrLanding()
{
	if(LocalFlightInformation.tracknumber == 1)
	{
		P(MutexTrack1);
		if((SharedMemory->Track1Used == 1) || (SharedMemory->NbPlaneAwaitingTrack1 > 0))
		{
			SharedMemory->NbPlaneAwaitingTrack1++;
			V(MutexTrack1);
			P(WaitTrack1);
			//Obligé de changer ces 2 variables durant la même utilisation de mutex car sinon la tour de contrôle avait le temps de faire 2 tours de boucle et déverouiller plusieurs fois la même
			P(MutexTrack1);
			SharedMemory->NbPlaneAwaitingTrack1--;
			SharedMemory->Track1Used = 1;
		}
		else
		{
			SharedMemory->Track1Used = 1;
		}
		V(MutexTrack1);

		if (LocalPlaneInformation.fromorto == 1)
			printf("Avion n°%d : atterissage piste 1\n",LocalPlaneInformation.num);
		else
			printf("Avion n°%d : décollage piste 1\n",LocalPlaneInformation.num);
		sleep((rand()%5)+5);
		printf("Avion n°%d a libéré la piste 1\n",LocalPlaneInformation.num);
		P(MutexTrack1);
		SharedMemory->Track1Used = 0;
		V(MutexTrack1);
	}
	else
	{
		P(MutexTrack2);
		if((SharedMemory->Track2Used == 1) || (SharedMemory->NbPlaneAwaitingTrack2 > 0))
		{
			SharedMemory->NbPlaneAwaitingTrack2++;
			V(MutexTrack2);
			P(WaitTrack2);
			//Changement des 2 variables durant la même utilisation de mutex pour éviter tout problème
			P(MutexTrack2);
			SharedMemory->NbPlaneAwaitingTrack2--;
			SharedMemory->Track2Used = 1;
		}
		else
		{
			SharedMemory->Track2Used = 1;
		}
		V(MutexTrack2);

		P(MutexTrack2);
		SharedMemory->Track2Used = 1;
		V(MutexTrack2);
		if (LocalPlaneInformation.fromorto == 1)
			printf("Avion n°%d : atterissage piste 2\n",LocalPlaneInformation.num);
		else
			printf("Avion n°%d : décollage piste 2\n",LocalPlaneInformation.num);
		sleep((rand()%5)+5);
		P(MutexTrack2);
		SharedMemory->Track2Used = 0;
		printf("Avion n°%d a libéré la piste 2\n",LocalPlaneInformation.num);
		V(MutexTrack2);
	}
}