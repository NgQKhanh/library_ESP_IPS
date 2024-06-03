#ifndef FINGERPRINTING_H
#define FINGERPRINTING_H

#include <database.h>

int determineLocation(int rssi[], int (*fingerprints)[NUM_AP]);
int euclideanDistance(int rssi1[], int rssi2[]);
bool isRssiValid (int* rssi);

#endif