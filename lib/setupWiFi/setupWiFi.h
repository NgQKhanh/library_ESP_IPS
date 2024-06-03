#ifndef SETUP_WIFI_H
#define SETUP_WIFI_H

#include <FS.h>
#include <SPIFFS.h>
#include <WiFiManager.h>

#ifndef ARDUINOJSON_H
#include <ArduinoJson.h>
#endif

void handleRoot(void);
void handleSave(void);
void startAPMode(void);
void readConfigFromJSON(void);
void saveConfigToJSON(String ssid, String password);
void initWiFi(void);
void connectToWiFi(String ssid, String password);

#endif