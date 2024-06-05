#ifndef MAIN_H
#define MAIN_H

#define SAMPLING_FREQ 3

//////////////////////////////////////////////////////////////////////////////////////
/// Global variables declare
///
//////////////////////////////////////////////////////////////////////////////////////
// const char* ssid = "TP-Link_E606";
// const char* password = "84209375";
// const char* ssid = "Khanh dep zai";
// const char* password = "1234567899";
const char* ssid = "Nania ";
const char* password = "Ngabon0976402";

const char* host = "192.168.1.6";
const int port = 3000;
const char* url = "/socket.io/?EIO=4";

int RSSI[NUM_AP];

SocketIOclient socketIO;

int count = 0;

//////////////////////////////////////////////////////////////////////////////////////
/// Functions declare
///
//////////////////////////////////////////////////////////////////////////////////////

void startScanRSSI(void *parameter);
void scanRSSI(void);
void processRSSIDataTask(void *parameter) ;
void connectToWiFi(String ssid, String password);

int determineLocation(int rssi[], int (*fingerprints)[NUM_AP]);
int euclideanDistance(int rssi1[], int rssi2[]);
bool isRssiValid (int* rssi, int size);
int isSSIDinDB(String ssid, const char* APname[], int numAP) ;

void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length);
void socketToServer(String api, int data);
void socketRSSIToServer(String event,const int* data, int len);
void httpToServer(String postData, String api);

void getAverage(int* avg_RSSI, int samplingFreq);
#endif