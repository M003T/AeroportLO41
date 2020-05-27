#ifndef MSGSTRUCTS_H
#define MSGSTRUCTS_H

//structure message de la tour de contrôle
typedef struct {
	long type;
	int tracknumber;
	char* routebrief;
	char* trackbrief;
	time_t liftoffhour;
	int maxdelay;
}	FlightInformation;

#endif