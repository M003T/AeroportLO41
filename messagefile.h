#ifndef MESSAGEFILE_H
#define MESSAGEFILE_H

#include <stdbool.h>
#include <time.h>

//Structure message infos initiales de l'avion
typedef struct {
	long type;
	char destination [40];
	bool fromorto;
	int size;
	int num;
}	PlaneInformationStruct;

//structure message infos de la tour de contrôle
typedef struct {
	long type;
	int tracknumber;
	char routebrief [50];
	char trackbrief [50];
	struct tm liftoffhour;
	int maxdelay;
}	FlightInformationStruct;

extern int msg_id;

void initmsgfile ();

void deletemsgfile ();

#endif