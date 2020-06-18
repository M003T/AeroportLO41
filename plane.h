#ifndef PLANE_H
#define PLANE_H

extern char* FranceDestinations[20];
extern char* EuropeDestinations[10];

extern SharedMemoryStruct *SharedMemory;
extern pid_t pid;

void sendPlaneInformation();
void initPlaneInformation();
void receiveFlightInformation();
void * plane (void *);
void takeOffOrLanding();
void testtimetogo();
void refreshOperatingMode();
void checkDelay();

#endif