#include <setupWiFi.h>
WebServer server(80);
const char* configFileName = "/config.json";
const int ssidAddr = 0;  // Starting address for storing the SSID in EEPROM.
const int passAddr = 64; // Starting address for storing the password in EEPROM.
const char* apSSID = "ESP32_AP";
String storedSSID;
String storedPass;

void handleRoot() 
{
  String html = "<!DOCTYPE html><html><head><style>";
  html += "body { font-family: Arial, sans-serif; background-color: #f4f4f4; margin: 0; padding: 0; }";
  html += "h1 { text-align: center; color: #333; }";
  html += "form { width: 300px; margin: 20px auto; background-color: #fff; padding: 20px; border-radius: 5px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }";
  html += "input[type='text'], input[type='password'], input[type='submit'] { width: 100%; padding: 10px; margin-bottom: 10px; border: 1px solid #ccc; border-radius: 5px; box-sizing: border-box; }";
  html += "input[type='submit'] { background-color: #4caf50; color: #fff; border: none; cursor: pointer; }";
  html += "input[type='submit']:hover { background-color: #45a049; }";
  html += "</style></head><body><h1>ESP32 Configuration</h1>";
  html += "<form method='post' action='/save'>";
  html += "SSID: <input type='text' name='ssid'><br>";
  html += "Password: <input type='password' name='password'><br>";
  html += "<input type='submit' value='Save'></form></body></html>";
  
  server.send(200, "text/html", html);
}

void handleSave() 
{
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  
  // Save SSID and Password into JSON file
  saveConfigToJSON(ssid, password);
  
  // Connect to WiFi
  WiFi.begin(ssid.c_str(), password.c_str());
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) 
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) 
  {
    Serial.println("Connected to WiFi!");
    server.send(200, "text/plain", "Configuration saved. Device connected to WiFi.");
  } 
  else 
  {
    Serial.println("Connection failed.");
    server.send(200, "text/plain", "Configuration failed. Please try again.");
  }
}

void startAPMode() 
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID);

  Serial.println("AP Mode started. Connect to ESP32's SSID to configure WiFi.");
  
  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.begin();
  
  while (true) 
  {
    server.handleClient();
  }
}

void readConfigFromJSON() 
{
  // Open config file
  File configFile = SPIFFS.open(configFileName, "r");
  if (!configFile) 
  {
    Serial.println("Failed to open config file");
    return;
  }

  // Read JSON file 
  size_t size = configFile.size();
  if (size > 1024) 
  {
    Serial.println("Config file size is too large");
    return;
  }

  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);

  // Parse JSON
  JsonDocument doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) 
  {
    Serial.println("Failed to parse config file");
    return;
  }

  // Read SSID and password JSON
  storedSSID = doc["ssid"].as<String>();
  storedPass = doc["password"].as<String>();

  Serial.println("Read configuration from JSON:");
  Serial.println("SSID: " + storedSSID);
  Serial.println("Password: " + storedPass);
}

void saveConfigToJSON(String ssid, String password) 
{
  // Create JSON object
  JsonDocument doc;
  doc["ssid"] = ssid;
  doc["password"] = password;

  // Open config
  File configFile = SPIFFS.open(configFileName, "w");
  if (!configFile) 
  {
    Serial.println("Failed to open config file for writing");
    return;
  }

  // Write JSON obj to file
  serializeJson(doc, configFile);
  configFile.close();

  Serial.println("Configuration saved to JSON:");
  Serial.println("SSID: " + ssid);
  Serial.println("Password: " + password);
}

void initWiFi()
{
  // Begin SPIFFS
  if (!SPIFFS.begin(true)) 
  {
    Serial.println("Failed to mount file system");
    return;
  }

  // Read SSID and PASS from config file
  readConfigFromJSON();

  // Connect WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(storedSSID.c_str(), storedPass.c_str());
  
  // Wait for connecting
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) 
  {
    delay(500);
    Serial.print(".");
  }
  
  // Check connection
  if (WiFi.status() != WL_CONNECTED) 
  {
    Serial.println("Connection failed. Starting AP mode.");
    startAPMode();
  } 
  else 
  {
    Serial.println("Connected to WiFi!");
  }
}

//////////////////////////////////////////////////////////////////////////////////////
/// @brief connect wifi in local mode
///
/// @param ssid 
/// @param password 
//////////////////////////////////////////////////////////////////////////////////////
void connectToWiFi(String ssid, String password) 
{
  Serial.print("Đang kết nối tới WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Đã kết nối tới WiFi. Địa chỉ IP: ");
  Serial.println(WiFi.localIP());
}