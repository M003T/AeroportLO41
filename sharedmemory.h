#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <stdbool.h>

typedef struct {
	bool exitrequested;
	int NbPlaneAwaitingInformation;
	int NbPlaneAwaitingTrack1;
	int NbPlaneAwaitingTrack2;
	int NbPlaneGoingTrack1;
	int NbPlaneGoingTrack2;
	bool Track1Used;
	bool Track2Used;
	int TrackNumberPlaneThatSentSignal;
}	SharedMemoryStruct;

extern int shm_id;

void initshm ();

void* getshm (void* );

void removeshm (void* );

void deleteshm ();

#endif