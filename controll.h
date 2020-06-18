#ifndef CONTROLL_H
#define CONTROLL_H

extern SharedMemoryStruct *SharedMemory;

PlaneInformationStruct PlaneInformation;
FlightInformationStruct FlightInformation;


void traitantSIGUSR1(int );
void traitantSIGUSR2(int );
void controll();
void initialMoveBarrier();
void receivePlaneInformation ();
void printPlaneInformation ();
void generateFlightInformation ();
void sendFlightInformation ();
void printFlightInformation ();
void testPlaneAwaitingInformation();
void testTrack1();
void testTrack2();
void addMinutes (int );
void randomBarrier(int );

#endif