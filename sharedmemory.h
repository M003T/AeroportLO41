#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <stdbool.h>

typedef struct {
	bool exitrequested;
	int NbPlaneAwaitingInformation;
}	SharedMemoryStruct;

extern int shm_id;

void initshm ();

void* getshm (void* );

void removeshm (void* );

void deleteshm ();

#endif