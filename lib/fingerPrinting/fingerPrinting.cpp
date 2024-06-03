
#ifndef FINGERPRINTING_H
#include <fingerPrinting.h>
#endif

#include <cmath>
#include <climits>

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Determine location based on RSSI fingerprinting using Euclidean distance
///
/// @param rssi Array of RSSI values for the current measurement
/// @param fingerprints 2D array of RSSI fingerprint data
/// @return  Index of the estimated location in the fingerprint data, or -1 if not found
//////////////////////////////////////////////////////////////////////////////////////
int determineLocation(int rssi[], int (*fingerprints)[NUM_AP]) 
{
  int location = -1;
  int minDistance = 99999;
 
  for (int i = 0; i < NUM_AP; i++) 
  {
    int distance = euclideanDistance(rssi, fingerprints[i]);
    if (distance < minDistance) 
    {
      minDistance = distance;
      location = i;
    }
  }

  // if (location != -1) 
  // {
  //   Serial.print("Estimated location: ");
  //   Serial.println(location);
  // } 
  // else 
  // {
  //   Serial.println("Location could not be determined.");
  // }
  return location;
}

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Calculate Euclidean distance between two RSSI vectors
///
/// @param rssi1 First RSSI vector
/// @param rssi2 Second RSSI vector
/// @return Euclidean distance
//////////////////////////////////////////////////////////////////////////////////////
int euclideanDistance(int rssi1[], int rssi2[]) 
{
  int result;
  for (int i = 0; i < NUM_AP; i++) 
  {
    result += pow(rssi1[i] - rssi2[i], 2);
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////////////////
/// @brief check if rssi data is valid
///
/// @param rssi 
/// @return true if valid, false if invalid
//////////////////////////////////////////////////////////////////////////////////////
bool isRssiValid (int* rssi)
{
  for(int i = 0; i<NUM_AP; i++)
  {
    if(rssi[i] > 0 || rssi[i] < -100) return false;
  }
  return true;
}