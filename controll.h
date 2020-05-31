#ifndef CONTROLL_H
#define CONTROLL_H

extern SharedMemoryStruct *SharedMemory;

PlaneInformationStruct PlaneInformation;
FlightInformationStruct FlightInformation;


void controll();
void receivePlaneInformation ();
void printPlaneInformation ();
void generateFlightInformation ();
void sendFlightInformation ();
void printFlightInformation ();
void testPlaneAwaitingInformation();
void testTrack1();
void testTrack2();

#endif