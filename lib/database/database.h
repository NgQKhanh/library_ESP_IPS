#ifndef DATABASE_H
#define DATABASE_H

#define NUM_AP 3
#define NUM_LOCATION 100

//const char* selectedSSIDs[NUM_AP] = {"Hust_C7", "Chu De", "Khanh dep zai"};
const char* selectedSSIDs[NUM_AP] = {"Nania ", "12345678910", "21 66A Trieu Khuc"};
int fingerprinting[NUM_LOCATION][NUM_AP] = 
{
    { -89, -76, -64 },
    { -88, -75, -54 },
    { -84, -77, -47 },
    { -79, -73, -50 },
    { -77, -63, -53 },
    { -82, -71, -73 },
    { -86, -67, -58 },
    { -82, -70, -50 },
    { -86, -75, -56 },
    { -85, -66, -47 },
    { -90, -72, -61 },
    { -90, -72, -66 },
    { -87, -78, -58 },
    { -85, -70, -65 },
    { -81, -69, -54 },   
};

#endif