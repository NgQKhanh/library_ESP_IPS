#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>

#include <setupWiFi.h>
#include <database.h>
#include "main.h"

//#define OFFLINE_PHASE

void setup() 
{
  Serial.begin(115200);

  //initWiFi();
  connectToWiFi(ssid, password);

  socketIO.begin(host, port, url);
  socketIO.onEvent(socketIOEvent);
  socketIO.loop();

  #ifndef OFFLINE_PHASE
    xTaskCreatePinnedToCore(startScanRSSI,"startScanRSSI", 10000, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(processRSSIDataTask, "processRSSIDataTask", 10000, NULL, 1, NULL, 0);
  #endif
}

void loop() 
{
  socketIO.loop();
}

//////////////////////////////////////////////////////////////////////////////////////
/// @brief  Task in core 1: Scan WiFi
///
/// @param parameter 
//////////////////////////////////////////////////////////////////////////////////////
void startScanRSSI(void *parameter) 
{
  while (1) 
  {
    //Serial.println("Scan task ==========================================");

    int numNetworks = WiFi.scanNetworks();

    if (numNetworks == 0) 
    {
      Serial.println("Doesn't found any network!");
      delay(100); 
      return;
    } 
    else 
    {
      for (int i = 0; i < numNetworks; i++) 
      {
        // Serial.print("  ");
        // Serial.print(WiFi.SSID(i));
        // Serial.print("  : ");
        // Serial.println(WiFi.RSSI(i));
        int ssidIndex = isSSIDinDB(WiFi.SSID(i), selectedSSIDs, NUM_AP);
        if (ssidIndex >= 0) 
        {
          RSSI[ssidIndex] = WiFi.RSSI(i);
          // Serial.print(WiFi.SSID(i)+"  :  ");
          // Serial.println(WiFi.RSSI(i));
        }
      }
    }
    WiFi.scanDelete();

    currentLocation ++;

    vTaskDelay(100);
  }
}

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Task in core 0: Process Data to determine location
///
/// @param parameter 
//////////////////////////////////////////////////////////////////////////////////////
void processRSSIDataTask(void *parameter) 
{
  while (1) 
  {
    // for(int i = 0; i<NUM_AP; i++)
    // {
    //   Serial.print(String(RSSI[i])+"  :");
    // }
    // Serial.println("");

    int location = determineLocation(RSSI,fingerprinting);

    int* pLocation= &currentLocation;

    socketToServer("ON_location", pLocation, 1);

    vTaskDelay(1000);
  }
}

//////////////////////////////////////////////////////////////////////////////////////
/// @brief http post data to server
///
/// @param data index of location 
//////////////////////////////////////////////////////////////////////////////////////
void httpToServer(String postData, String api) 
{
  if (WiFi.status() == WL_CONNECTED) 
  {
    HTTPClient http;
    
    // Your server address and endpoint
    char url[100];
    sprintf(url, "http://%s:%d/%s", host, port, api);
    http.begin(url);
  
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpResponseCode = http.POST(postData);
    
    // If successful, print the response
    if (httpResponseCode > 0) 
    {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } 
    else 
    {
      Serial.print("Error in sending POST request. HTTP error code: ");
      Serial.println(httpResponseCode);
    }
    
    // Close the connection
    http.end();
    delay(50);
  }
}

//////////////////////////////////////////////////////////////////////////////////////
/// @brief    Check if this Wifi is in database
///
/// @param ssid WiFi name
/// @param APname array of AP name in db
/// @param numAP number of AP in db
/// @return WiFi index in db; -1 if not in db
//////////////////////////////////////////////////////////////////////////////////////
int isSSIDinDB(String ssid, const char* APname[], int numAP) 
{
  for (int i = 0; i < numAP; i++) 
  {
    if (strcmp(ssid.c_str(), APname[i]) == 0) {
      return i;
    }
  }
  return -1;
}

//////////////////////////////////////////////////////////////////////////////////////
/// @brief      handle event when have socket message
/// @param type 
/// @param payload 
/// @param length 
//////////////////////////////////////////////////////////////////////////////////////
void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) 
{
    switch(type) {
        case sIOtype_DISCONNECT:
            Serial.printf("[IOc] Disconnected!\n");
            break;

        case sIOtype_CONNECT:
            Serial.printf("[IOc] Connected to url: %s\n", payload);
            socketIO.send(sIOtype_CONNECT, "/");
            break;

        case sIOtype_EVENT:
          {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload, length);
            if(error) 
            {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
                return;
            }
            String eventName = doc[0];
            if(eventName.equals("STE"))
            {
              Serial.println("STE");
              int avg_RSSI[NUM_AP] = {};
              //getAverage(avg_RSSI, SAMPLING_FREQ);

              String postData;
              for (int i = 0; i< NUM_AP-1; i++)
              {
                postData = postData + "AP" +String(i) +"="+ String(avg_RSSI[i])+"&";
              }
              postData = postData +  "AP" +String(NUM_AP-1) +"="+ String(avg_RSSI[NUM_AP-1]);
              Serial.println(postData);
              
              httpToServer(postData,"dataGathering");
            }
            break;
          }
        case sIOtype_ACK:
            Serial.printf("[IOc] get ack: %u\n", length);
            break;
        case sIOtype_ERROR:
            Serial.printf("[IOc] get error: %u\n", length);
            break;
        case sIOtype_BINARY_EVENT:
            Serial.printf("[IOc] get binary: %u\n", length);
            break;
        case sIOtype_BINARY_ACK:
            Serial.printf("[IOc] get binary ack: %u\n", length);
            break;
    }
}

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
/// @param size
/// @return true if valid, false if invalid
//////////////////////////////////////////////////////////////////////////////////////
bool isRssiValid (int* rssi, int size)
{
  for(int i = 0; i<size; i++)
  {
    if(rssi[i] > 0 || rssi[i] < -100) return false;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////
/// @brief Socket send data to server
///
/// @param event name of event send to server
/// @param data  data array
/// @param len length of data array
//////////////////////////////////////////////////////////////////////////////////////
void socketToServer(String event,const int* data, int len)
{
  // Create json array: [EST,param]
    JsonDocument doc; 
    JsonArray array = doc.to<JsonArray>();
    array.add("ETS");
    JsonObject param = array.add<JsonObject>();

  // Create json object param: {event, data}
    param["event"] = event;
    JsonObject eventData = param["data"].to<JsonObject>();
    for(int i = 0; i<len; i++)
    {
      eventData["data_"+String(i)] = data[i];
    }

    String output;
    serializeJson(doc, output);
    socketIO.sendEVENT(output);
    //Serial.println("Output: " + String(output));
}
