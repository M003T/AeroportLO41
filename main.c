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
#include "plane.h"
#include "msgstructs.h"
#include "semaphore.h"

int exitrequested;
int msgid;

char* FranceDestinations[20] = {"Paris - Charles de Gaulle","Paris - Orly","Nice - Côte d’Azur","Lyon - Saint Exupéry","Marseille - Provence","Toulouse - Blagnac","Bâle - Mulhouse - Fribourg","Bordeaux - Mérignac","Nantes - Atlantique","Paris - Beauvais-Tillé","Guadeloupe - Pôle Caraïbes", "La Réunion - R. Garros","Lille - Lesquin","Martinique - A. Césaire","Montpellier - Méditerranée","Ajaccio - Napoléon-Bonaparte","Bastia - Poretta","Tahiti - Faaa","Strasbourg","Brest - Bretagne"};
char* EuropeDestinations[10] = {"Londres - Heathrow","Amsterdam - Schiphol","Francfort - Rhin/Main","Madrid/Barajas - Adolfo-Suárez","Barcelone - El Prat","Istanbul","Moscou - Cheremetievo","Munich - Franz-Josef-Strauß","Londres - Gatwick","Rome Fiumicino - Léonard-de-Vinci"};
int NbPlaneAwaitingInformation;

FlightInformation FlightInformationMsg;

void traitantSIGINT(int num) {

	if (num!=SIGINT)
    perror("Erreur SigInt\n");

	exitrequested = 1;
	}

void attenterandom(int n) {
   sleep(rand() % n);
}

void * plane (void * arg)
{
	//Recuperation de son numero
	int num = (int) (intptr_t) arg;
	
	//Choix aléatoire destination	
	int destinationnumber = rand() % 30;
	char* destination = (char*) malloc(50);
	if (destinationnumber <= 19)
		strcpy(destination,FranceDestinations[destinationnumber]);
	else
		strcpy(destination,EuropeDestinations[destinationnumber-20]);

	printf("Avion n°%d, destination %s\n",num,destination);
	//Attente Informations de vol

	P(MutexNbPlaneAwaitingInformation);
	NbPlaneAwaitingInformation++;
	if(NbPlaneAwaitingInformation > 1)
	{
		V(MutexNbPlaneAwaitingInformation);
		P(WaitFlightInformation);
	}
	else
		V(MutexNbPlaneAwaitingInformation);
	//Reception Message Informations de vol

	NbPlaneAwaitingInformation--;
	if ((msgrcv(msgid, &FlightInformationMsg, sizeof(FlightInformation) - sizeof(long), 1, 0)) == -1) 
	{
	    perror("Erreur de lecture msg\n");
	    pthread_exit((void *) 1);
	}
	printf("Départ voie %d et délai max %d\n",FlightInformationMsg.tracknumber,FlightInformationMsg.maxdelay);
	
	free(destination);
	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
	srand(time(0));

	if (argc-1 != 1) {
        perror("Veuillez passer en argument le nombre total d'avions pour cette simulations\n");
        return 1;
        }

    //Initialisation variables partagées
    NbPlaneAwaitingInformation = 0;

	//Réecriture signal Ctrl-C pour que les objets IPC soient correctement supprimés en cas d'arrêt prématuré
	exitrequested = 0;
	signal(SIGINT,traitantSIGINT);

	//Initialisation sémaphores
    if ((sem_id = initsem(SKEY)) < 0)
		return(1);

	//Initialisation file de message
	if ((msgid = msgget(SKEY, IFLAGS)) == -1) 
	{
		perror("Erreur de creation de la file\n");
		return(1);
    }


	pid_t pid;
	pid = fork();

	if (pid == 0)
	{
		controll();
		exit(0);
	}
	else
	{
		int NbPlaneMax=atoi(argv[1]);
		int i=1;
		pthread_t thr[NbPlaneMax];

    	while (i<=NbPlaneMax && exitrequested != 1)
		{
			if (pthread_create(&thr[i-1], NULL, plane, (void *) (intptr_t) i) != 0)
			{
				perror("Erreur Creation Thread\n");
				return(1);
			}
    		attenterandom(5);
    		i++;
		}
		int status;
		pthread_join(thr[i-2],NULL);
		waitpid(pid,&status,0);
		if (liberesem() < 0)
   			return(1);
	}
	
	return(0);
}