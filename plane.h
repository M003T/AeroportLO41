#ifndef PLANE_H
#define PLANE_H

extern char* FranceDestinations[20];
extern char* EuropeDestinations[10];

extern SharedMemoryStruct *SharedMemory;

FlightInformationStruct FlightInformation;
PlaneInformationStruct PlaneInformation;

void sendPlaneInformation();
void initPlaneInformation();
void receiveFlightInformation();
void * plane (void *);

#endif