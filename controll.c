#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "sharedmemory.h"
#include "messagefile.h"
#include "controll.h"
#include "semaphore.h"
#include "constants.h"
#include "randompart.h"

PlaneInformationStruct PlaneInformation;
FlightInformationStruct FlightInformation;

int barrier1count;
int barrier2count;

void controll ()
{
	//Les signaux sont reliés à leur traitant pour que les barrières soient enlevées quand les signaux envoyés par les avions passant en mode Assuré ou Urgent sont reçu
	signal(SIGUSR1, traitantSIGUSR1);
	signal(SIGUSR2, traitantSIGUSR2);

	barrier1count = 0;
	barrier2count = 0;

	//Sémaphores Mutex d'accès aux variables monitrice afin que le programme ne s'arrête pas tant que des avions sont encore présent
	P(MutexNbPlaneAwaitingInformation);
	P(MutexTrack1);
	P(MutexTrack2);
	//La boucle de la tour de contrôle continue tant que l'exit n'est pas requested ET tant que des avions sont encore présents pour éviter de laisser des threads bloqués en attente
	while ((SharedMemory->exitrequested == 0) || (SharedMemory->NbPlaneAwaitingInformation > 0) || (SharedMemory->NbPlaneAwaitingTrack1 > 0) || (SharedMemory->NbPlaneAwaitingTrack2 > 0) || (SharedMemory->Track1Used == 1) || (SharedMemory->Track2Used == 1) || (SharedMemory->NbPlaneGoingTrack1 > 0) || (SharedMemory->NbPlaneGoingTrack2 > 0))
	{	
		V(MutexNbPlaneAwaitingInformation);
		V(MutexTrack1);
		V(MutexTrack2);

		//Si avions attendent information alors on leur envoie
		testPlaneAwaitingInformation();

		//Obstacle aléatoire sur les 2 pistes ?
		randomBarrier(1);
		randomBarrier(2);

		//Si avions attendent sur la piste 1 alors on les déverouille
		testTrack1();
		
		//Si avions attendent sur la piste 2 alors on les déverouille
		testTrack2();

		sleep(1);
		//Sleep nécessaire pour éviter que la tour de contrôle ait le temps de faire un tour de boucle complet et déverouiller un 2ème avion avant que ce dernier ait le temps de décoller, ainsi que pour éviter la création de trop nombreux obstacles
		
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
		printf("Avion n°%d : En attente d'informations : Gros Porteur ",PlaneInformation.num);
	else
		printf("Avion n°%d : En attente d'informations : Petit Calibre ",PlaneInformation.num);
	if (PlaneInformation.fromorto == 1)
		printf("en provenance de %s\n",PlaneInformation.destination);
	else
		printf("à destination de %s\n",PlaneInformation.destination);
}

void generateFlightInformation ()
{
	FlightInformation.type = 2;

	if (PlaneInformation.size == 1)
	{
		P(MutexTrack1);
		P(MutexTrack2);
		//Si la petite piste est en surcharge par rapport à la grande, les avions petit calibre décollent sur la grande
		if( (SharedMemory->NbPlaneAwaitingTrack1 + SharedMemory->NbPlaneGoingTrack1 + ControllNumberBeforeSmallOnTrack1) <= (SharedMemory->NbPlaneAwaitingTrack2 + SharedMemory->NbPlaneGoingTrack2) )
		{
			FlightInformation.tracknumber = 1;
			printf("Tour de contrôle : Piste 2 : Surcharge, avion petit calibre envoyé vers piste 1\n");
		}	
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
	//Création du mode de fonctionnement en fonction du niveau de carburant restant
	if ( PlaneInformation.fuellvl < PlaneFuelUrgentThreshold)
	{
		FlightInformation.operatingmode = 2;
	}
	else
	{
		if( PlaneInformation.fuellvl < PlaneFuelInsuredThreshold+1)
		{
			FlightInformation.operatingmode = 1;
		}
		else
			FlightInformation.operatingmode = 0;
	}
	//Création de l'heure de départ et du délai max en fonction du mode de fonctionnement de l'avion
	time_t mytime = time(NULL);
	FlightInformation.takeofforlandinghour = *localtime( &mytime );
	if ( FlightInformation.operatingmode == 0)
	{
		addMinutes(2);
		FlightInformation.maxdelay = 3;
	}
	else if( FlightInformation.operatingmode == 1)
	{
		addMinutes(1);
		FlightInformation.maxdelay = 2;
	}
	else
	{
		//Sinon en mode urgent (2) l'heure de départ reste l'heure actuelle, c'est à dire départ immédiat
		FlightInformation.maxdelay = 1;
	}
}

void addMinutes (int minutestoadd)
{
	//Cette fonction permet de s'assurer que l'heure reste valide, si le champ des minutes dépasse 60 alors on augmente d'1h et on enlève 60 min
	if ( (FlightInformation.takeofforlandinghour.tm_min + minutestoadd) >=60 )
	{
		FlightInformation.takeofforlandinghour.tm_hour++;
		FlightInformation.takeofforlandinghour.tm_min = FlightInformation.takeofforlandinghour.tm_min + minutestoadd - 60;
	}
	else
	{
		FlightInformation.takeofforlandinghour.tm_min = FlightInformation.takeofforlandinghour.tm_min + minutestoadd;
	}
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
	printf("Avion n°%d : Informations de vol : ",PlaneInformation.num);
	if (PlaneInformation.fromorto == 1)
		printf("Piste %d : Arrivée prévue",FlightInformation.tracknumber);
	else
		printf("Piste %d : Départ prévu",FlightInformation.tracknumber);

	if (FlightInformation.takeofforlandinghour.tm_min < 10)
		printf(" à %d:0%d, délai max %d min",FlightInformation.takeofforlandinghour.tm_hour,FlightInformation.takeofforlandinghour.tm_min,FlightInformation.maxdelay);
	else
		printf(" à %d:%d, délai max %d min",FlightInformation.takeofforlandinghour.tm_hour,FlightInformation.takeofforlandinghour.tm_min,FlightInformation.maxdelay);

	if (FlightInformation.operatingmode == 0)
		printf(", avion en mode Normal\n");
	if (FlightInformation.operatingmode == 1)
		printf(", avion en mode Assuré\n");
	if (FlightInformation.operatingmode == 2)
		printf(", avion en mode Urgent\n");
}

void testPlaneAwaitingInformation()
{
	//Cette fonction correspond à tout la boucle correspondant à l'envoi d'un message depuis l'avion puis à son message retour : si un avion est actuellement en attente pour envoyer des informations, alors on le débloque, on les reçoit, les affiche puis on génère les information de retour qu'on affiche puis envoie, avant d'utiliser initialMoveBarrier pour enlever les obstacles si l'avion qui vient d'être crée est en mode Assuré ou Urgent
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
		initialMoveBarrier();
	}
	else
		V(MutexNbPlaneAwaitingInformation);
}

void testTrack1()
{
	//Si un avion attend la piste 1 et que cette derniere est libre, alors on le débloque
	P(MutexTrack1);
	if(SharedMemory->NbPlaneAwaitingTrack1 > 0 && SharedMemory->Track1Used == 0 )
	{
		V(MutexTrack1);

		V(WaitTrack1);
	}
	else
		V(MutexTrack1);
}

void testTrack2()
{
	//Si un avion attend la piste 2 et que cette derniere est libre, alors on le débloque
	P(MutexTrack2);
	if(SharedMemory->NbPlaneAwaitingTrack2 > 0 && SharedMemory->Track2Used == 0 )
	{
		V(MutexTrack2);
		
		V(WaitTrack2);
	}
	else
		V(MutexTrack2);
}

void randomBarrier(int tracknumber)
{
	//Cette fonction tente de générer alétoirement un obstacle sur une des 2 pistes à chaque passage de la boucle
	if(tracknumber == 1)
	{	
		P(MutexBarrier1);
		P(MutexTrack1);
		//Si il n'y a pas déjà un obstacle et que la piste est vide alors ->
		if(barrier1count == 0 && SharedMemory->Track1Used == 0)
		{
			//1 chance sur X d'avoir un obstacle sur la piste
			if ( rand()%ControllBarrierOdds == 1)
			{
				SharedMemory->Track1Used = 1;
				barrier1count = randompart(ControllBarrierDelay);

				//Un obstacle bloque la piste 1 pendant X passages au compteur, utilisation d'un compteur obligatoire car un sleep mettrait toute la tour de contrôle en pause
				printf("Tour de contrôle : Piste 1 : Obstacle inconnu\n");
			}
		}
		else if (barrier1count > 0)
		//Si il y'a déjà un obstacle, on diminue son compteur et si ce dernier atteint 0, la piste est libérée
		{
			barrier1count--;
			if(barrier1count == 0)
			{
				SharedMemory->Track1Used = 0;

				printf("Tour de contrôle : Piste 1 : Obstacle enlevé\n");
			}
		}
		V(MutexTrack1);
		V(MutexBarrier1);
	}
	else
	{
		//Pareil pour la piste 2
		P(MutexTrack2);
		P(MutexBarrier2);
		if(barrier2count == 0 && SharedMemory->Track2Used == 0)
		{
			if ( rand()%ControllBarrierOdds == 1)
			{
				SharedMemory->Track2Used = 1;
				barrier2count = randompart(ControllBarrierDelay);

				printf("Tour de contrôle : Piste 2 : Obstacle inconnu\n");
			}
		}
		else if (barrier2count > 0)
		{
			barrier2count--;
			if(barrier2count == 0)
			{
				SharedMemory->Track2Used = 0;

				printf("Tour de contrôle : Piste 2 : Obstacle enlevé\n");
			}
		}
		V(MutexTrack2);
		V(MutexBarrier2);
	}
}

void initialMoveBarrier()
{
	//Enlevement initial des obstacles en fonction du mode de l'avion généré et de sa piste
	if( FlightInformation.operatingmode == 2 )
	{
		P(MutexPlaneSignal);
		SharedMemory->TrackNumberPlaneThatSentSignal = FlightInformation.tracknumber;
		V(MutexPlaneSignal);

		traitantSIGUSR2(12);
	}
	else if( FlightInformation.operatingmode == 1 )
	{
		P(MutexPlaneSignal);
		SharedMemory->TrackNumberPlaneThatSentSignal = FlightInformation.tracknumber;
		V(MutexPlaneSignal);

		traitantSIGUSR1(10);
	}
}

void traitantSIGUSR1(int num) //Signal envoyé par un avion passant en mode Assuré, sa piste est passée via la structure SharedMemory et protégée par des variables Mutex, obligé car SIGUSR est limité à 2 impossible d'en créer 4 différents sans réecrire les signaux système
{
	if (num!=SIGUSR1)
	{
		perror("Erreur signal SigUsr1");
		//exit(EXIT_FAILURE);
	}
	else
	{
		P(MutexPlaneSignal);
		if(SharedMemory->TrackNumberPlaneThatSentSignal == 1)
		{
			SharedMemory->TrackNumberPlaneThatSentSignal = 0;
			V(MutexPlaneSignal);

			P(MutexBarrier1);
			//Si il y'a un obstacle avec un temps minimum restant, alors on diminue ce temps
			if (barrier1count > ControllBarrierDelayReduceWhenInsuredPlane)
			{
				barrier1count = barrier1count - ControllBarrierDelayReduceWhenInsuredPlane;
				printf("Tour de contrôle : Piste 1 : Envoi équipe pour enlever rapidement obstacle\n");
			}
			V(MutexBarrier1);
		}
		else if(SharedMemory->TrackNumberPlaneThatSentSignal == 2)
		{
			//Pareil pour la piste 2
			SharedMemory->TrackNumberPlaneThatSentSignal = 0;
			V(MutexPlaneSignal);

			P(MutexBarrier2);
			if (barrier2count > ControllBarrierDelayReduceWhenInsuredPlane)
			{
				barrier2count = barrier2count - ControllBarrierDelayReduceWhenInsuredPlane;
				printf("Tour de contrôle : Piste 2 : Envoi équipe pour enlever rapidement obstacle\n");
			}
			V(MutexBarrier2);
		}
	}
}

void traitantSIGUSR2(int num) //Signal envoyé par un avion passant en mode Urgent
{
	if (num!=SIGUSR2)
	{
		perror("Erreur signal SigUsr2");
		//exit(EXIT_FAILURE);
	}
	else
	{
		P(MutexPlaneSignal);
		if(SharedMemory->TrackNumberPlaneThatSentSignal == 1)
		{
			SharedMemory->TrackNumberPlaneThatSentSignal = 0;
			V(MutexPlaneSignal);

			P(MutexBarrier1);
			//Si un obstacle est présent alors son compteur est amené directement à 1 et il sera donc enlevé par l'équipe au prochain tour de boucle
			if (barrier1count > 0)
			{
				barrier1count = 1;
				printf("Tour de contrôle : Piste 1 : Envoi équipe pour enlever obstacle instantanément\n");
			}
			V(MutexBarrier1);
		}
		else if(SharedMemory->TrackNumberPlaneThatSentSignal == 2)
		{
			//Pareil pour la piste 2
			SharedMemory->TrackNumberPlaneThatSentSignal = 0;
			V(MutexPlaneSignal);

			
			P(MutexBarrier2);
			if (barrier2count > 0)
			{
				barrier2count = 1;
				printf("Tour de contrôle : Piste 2 : Envoi équipe pour enlever obstacle instantanément\n");
			}
			V(MutexBarrier2);
		}
	}
}