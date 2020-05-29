#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdint.h>

#include "sharedmemory.h"
#include "messagefile.h"
#include "controll.h"
#include "plane.h"
#include "semaphore.h"

char* FranceDestinations[20] = {"Paris - Charles de Gaulle","Paris - Orly","Nice - Côte d’Azur","Lyon - Saint Exupéry","Marseille - Provence","Toulouse - Blagnac","Bâle - Mulhouse - Fribourg","Bordeaux - Mérignac","Nantes - Atlantique","Paris - Beauvais-Tillé","Guadeloupe - Pôle Caraïbes", "La Réunion - R. Garros","Lille - Lesquin","Martinique - A. Césaire","Montpellier - Méditerranée","Ajaccio - Napoléon-Bonaparte","Bastia - Poretta","Tahiti - Faaa","Strasbourg","Brest - Bretagne"};
char* EuropeDestinations[10] = {"Londres - Heathrow","Amsterdam - Schiphol","Francfort - Rhin/Main","Madrid/Barajas - Adolfo-Suárez","Barcelone - El Prat","Istanbul","Moscou - Cheremetievo","Munich - Franz-Josef-Strauß","Londres - Gatwick","Rome Fiumicino - Léonard-de-Vinci"};

SharedMemoryStruct *SharedMemory;

void traitantSIGINT(int num) 
{
	if (num!=SIGINT)
	{
		perror("Erreur signal SigInt");
		//exit(EXIT_FAILURE);
	}
	else
	{
		SharedMemory->exitrequested = 1;
	}
}

void attenterandom(int n) {
	sleep(rand() % n);
}

int main(int argc, char *argv[]) 
{
	srand(time(0));

	//Recuperation Nombre Max d'avions passée en argument du programme
	if (argc-1 != 1) 
	{
        perror("Erreur : Veuillez passer en argument le nombre total d'avions pour cette simulations");
        return(-1);
    }
    int NbPlaneMax=atoi(argv[1]);

    //Initialisation Objets IPC
    initsem();
    initmsgfile();
    initshm();

    //Attachement segment mémoire partagée et initialisation variables
	SharedMemory = (SharedMemoryStruct*) getshm(SharedMemory);
	SharedMemory->exitrequested = 0;
	SharedMemory->NbPlaneAwaitingInformation = 0;

	//Réecriture signal Ctrl-C pour que les objets IPC soient correctement supprimés en cas d'arrêt prématuré
	signal(SIGINT,traitantSIGINT);

    //Création processus fils (tour de contrôle)
    pid_t pid;
	pid = fork();
	if (pid == 0)
	{
		controll();

		//Détachement mémoire partagée et exit
		removeshm((void *) SharedMemory);
		exit(0);
	}
	else
	{
		int i = 0;
		pthread_t thr[NbPlaneMax];

		//Boucle de création des threads (avions)
    	while ( (i<NbPlaneMax) && (SharedMemory->exitrequested == 0) )
		{
			if (pthread_create(&thr[i], NULL, plane, (void *) (intptr_t) i) != 0)
			{
				perror("Erreur Creation Thread");
				//return(-1);
			}
    		attenterandom(5);
    		i++;
		}
		
		//Fermeture Threads, Processus fils et Objets IPC et détachement mémoire partagée
		SharedMemory->exitrequested = 1;
		int j;
		for (j=0;j<i;j++)
			pthread_join(thr[j],NULL);
		int status;
		waitpid(pid,&status,0);
		if (status != 0)
		{
			perror("Erreur Fermeture Processus Fils");
			//return(-1);
		}

		removeshm( SharedMemory);

		deletesem();
		deleteshm();
		deletemsgfile();
	}
	
	return(0);
}