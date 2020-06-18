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
#include <sys/types.h>
#include <signal.h>

#include "semaphore.h"
#include "sharedmemory.h"
#include "messagefile.h"
#include "plane.h"
#include "constants.h"
#include "randompart.h"

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

	LocalPlaneInformation.fromorto = rand()%2;

	if (rand()%PlaneGenerateBigOdds == 0)
		LocalPlaneInformation.size = 2;
	else
		LocalPlaneInformation.size = 1;

	LocalPlaneInformation.fuellvl = (rand()%PlaneFuelLvlRange)+1;
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
	P(MutexPlanesWaiting);
	SharedMemory->PlanesWaiting++;
	V(MutexPlanesWaiting);

	time_t timestruct = time(NULL);
	struct tm actualtime = *localtime( &timestruct );
	//Tant que l'heure actuelle est différent de l'heure de décollage, ou que la minute actuelle est inférieure à la minute de décollage, ou que le mode de l'avion reste normal ou assuré, on attend, et le carburant diminue durant l'attente
	while( ((LocalFlightInformation.takeofforlandinghour.tm_hour != actualtime.tm_hour) || (actualtime.tm_min < LocalFlightInformation.takeofforlandinghour.tm_min)) && LocalFlightInformation.operatingmode < 2 )
	{
		sleep(PlaneLoseFuelDelay);
		timestruct = time(NULL);
		actualtime = *localtime( &timestruct );
		LocalPlaneInformation.fuellvl--;
	}
	
	P(MutexPlanesWaiting);
	SharedMemory->PlanesWaiting--;
	V(MutexPlanesWaiting);
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
			printf("Avion n°%d : Piste 1 : atterissage\n",LocalPlaneInformation.num);
		else
			printf("Avion n°%d : Piste 1 : décollage\n",LocalPlaneInformation.num);
		sleep(randompart(PlaneTakeOfOrLandingDelay));
		printf("Avion n°%d : Piste 1 : libérée\n",LocalPlaneInformation.num);
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
			printf("Avion n°%d : Piste 2 : atterissage\n",LocalPlaneInformation.num);
		else
			printf("Avion n°%d : Piste 2 : décollage\n",LocalPlaneInformation.num);
		sleep(randompart(PlaneTakeOfOrLandingDelay));
		P(MutexTrack2);
		SharedMemory->Track2Used = 0;
		printf("Avion n°%d : Piste 2 : libérée \n",LocalPlaneInformation.num);
		V(MutexTrack2);
	}
}

void refreshOperatingMode()
{
	//Actualisation du mode de fonctionnement de l'Avion en fonction du carburant restant
	if( LocalPlaneInformation.fuellvl < PlaneFuelUrgentThreshold+1 && LocalFlightInformation.operatingmode == 1)
	{
		printf("Avion n°%d : En attente piste %d : Passage en Mode Urgent\n",LocalFlightInformation.tracknumber,LocalPlaneInformation.num);
		LocalFlightInformation.operatingmode = 2;

		//Envoi du signal pour un avion assuré et utilisation de la mémoire partagée pour que la tour de contrôle récupère le numéro de piste de l'avion envoyant le signal
		if(LocalFlightInformation.tracknumber == 1)
		{
			P(MutexPlaneSignal);

			//Attente au cas où un avion vient déjà d'envoyer un signal
			while(SharedMemory->TrackNumberPlaneThatSentSignal != 0)
			{
				V(MutexPlaneSignal);
				sleep(1);
				P(MutexPlaneSignal);
			}

			SharedMemory->TrackNumberPlaneThatSentSignal = 1;
			V(MutexPlaneSignal);
			kill(pid,SIGUSR2);
		}
		else
		{
			P(MutexPlaneSignal);

			while(SharedMemory->TrackNumberPlaneThatSentSignal != 0)
			{
				V(MutexPlaneSignal);
				sleep(1);
				P(MutexPlaneSignal);
			}

			SharedMemory->TrackNumberPlaneThatSentSignal = 2;
			V(MutexPlaneSignal);
			kill(pid,SIGUSR2);
		}
	}
	else if( LocalPlaneInformation.fuellvl < PlaneFuelInsuredThreshold+1 && LocalFlightInformation.operatingmode == 0)
	{
		printf("Avion n°%d : En attente piste %d : Passage en Mode Assuré\n",LocalPlaneInformation.num,LocalFlightInformation.tracknumber);
		LocalFlightInformation.operatingmode = 1;

		if(LocalFlightInformation.tracknumber == 1)
		{
			P(MutexPlaneSignal);

			while(SharedMemory->TrackNumberPlaneThatSentSignal != 0)
			{
				V(MutexPlaneSignal);
				sleep(1);
				P(MutexPlaneSignal);
			}

			SharedMemory->TrackNumberPlaneThatSentSignal = 1;
			V(MutexPlaneSignal);
			kill(pid,SIGUSR1);
		}
		else
		{
			P(MutexPlaneSignal);

			while(SharedMemory->TrackNumberPlaneThatSentSignal != 0)
			{
				V(MutexPlaneSignal);
				sleep(1);
				P(MutexPlaneSignal);
			}
			
			SharedMemory->TrackNumberPlaneThatSentSignal = 2;
			V(MutexPlaneSignal);
			kill(pid,SIGUSR1);
		}
	}
}