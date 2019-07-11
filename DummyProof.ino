//Bronnen: 
//https://github.com/techiesms/Geolocation
//https://www.geekstips.com/android-push-notifications-esp8266-arduino-tutorial/
//www.pushingbox.com
//www.pushbullet.com

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// PushingBox scenario DeviceId code and API
String deviceId = "v9EDEFE678B4E04F";
const char* logServer = "api.pushingbox.com";
#define WIFI_SSID     "iPhone Julian"
#define WIFI_PASSWORD "schoenendoos"

#define BUTTON_PIN  D1
#define BUTTONLOW_PIN D0

//Google API
const char* Host = "www.googleapis.com";
String thisPage = "/geolocation/v1/geolocate?key=";
String key = "AIzaSyCBWG0dnvEiTfBoJ7rMghvnuB7P1tB5kbg";

int status = WL_IDLE_STATUS;
String jsonString = "{\n";

double latitude    = 0.0;
double longitude   = 0.0;
double accuracy    = 0.0;

void sendNotification(String message){ //code om notificatie te sturen

  Serial.println("- connecting to Home Router SID: " + String(WIFI_SSID));
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  
  WiFiClient client;

  Serial.println("- connecting to pushing server: " + String(logServer));
  if (client.connect(logServer, 80)) {
    Serial.println("- succesfully connected");
    
    String postStr = "devid=";
    postStr += String(deviceId);
    postStr += "&message_parameter=";
    postStr += String(message);
    postStr += "\r\n\r\n";
    
    Serial.println("- sending data...");
    
    client.print("POST /pushingbox HTTP/1.1\n");
    client.print("Host: api.pushingbox.com\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
  client.stop();
  Serial.println("- stopping the client");
  
}



void setup() {
  Serial.begin(9600);
   Serial.println("Start");
  
  // connecten met WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
   WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("Setup done");
  
  // Opnieuw verbinden
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nconnected: ");
  Serial.println(WiFi.localIP());

  pinMode(BUTTONLOW_PIN, OUTPUT);
  digitalWrite(BUTTONLOW_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
}

void loop() {
if(digitalRead(BUTTON_PIN) == LOW){ // check of de button ingedrukt is
    Serial.print("Button pressed");
    location();
    
  Serial.println("Test geslaagd");

 while(digitalRead(BUTTON_PIN) == LOW){
      Serial.print(".");
      delay(500);
    }

   }
  }

  void location() { // code om locatie te berekenen
  char bssid[6];
  DynamicJsonBuffer jsonBuffer;
  Serial.println("scan start");
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found...");

    Serial.println(" ");
  }
  // now build the jsonString...
  jsonString = "{\n";
  jsonString += "\"homeMobileCountryCode\": 234,\n"; // this is a real UK MCC
  jsonString += "\"homeMobileNetworkCode\": 27,\n";  // and a real UK MNC
  jsonString += "\"radioType\": \"gsm\",\n";         // for gsm
  jsonString += "\"carrier\": \"Vodafone\",\n";      // associated with Vodafone
  jsonString += "\"wifiAccessPoints\": [\n";
  for (int j = 0; j < n; ++j)
  {
    jsonString += "{\n";
    jsonString += "\"macAddress\" : \"";
    jsonString += (WiFi.BSSIDstr(j));
    jsonString += "\",\n";
    jsonString += "\"signalStrength\": ";
    jsonString += WiFi.RSSI(j);
    jsonString += "\n";
    if (j < n - 1)
    {
      jsonString += "},\n";
    }
    else
    {
      jsonString += "}\n";
    }
  }
  jsonString += ("]\n");
  jsonString += ("}\n");


   Serial.println("");

  WiFiClientSecure client;

  //Connect to the client and make the api call
  Serial.print("Requesting URL: ");
  Serial.println("https://" + (String)Host + thisPage + "AIzaSyCYNXIYINPmTNIdusMjJloS4_BXSOff1_g");
  Serial.println(" ");
  if (client.connect(Host, 443)) {
    Serial.println("Connected");
    client.println("POST " + thisPage + key + " HTTP/1.1");
    client.println("Host: " + (String)Host);
    client.println("Connection: close");
    client.println("Content-Type: application/json");
    client.println("User-Agent: Arduino/1.0");
    client.print("Content-Length: ");
    client.println(jsonString.length());
    client.println();
    client.print(jsonString);
    delay(500);
  }

  //Read and parse all the lines of the reply from server
  while (client.available()) {
    String line = client.readStringUntil('\r');
     JsonObject& root = jsonBuffer.parseObject(line);
    if (root.success()) {
      latitude    = root["location"]["lat"];
      longitude   = root["location"]["lng"];
      accuracy   = root["accuracy"];
    }
  }
   Serial.println("closing connection");
  Serial.println();
  client.stop();

  Serial.print("Latitude = ");
  Serial.println(latitude, 6);
  Serial.print("Longitude = ");
  Serial.println(longitude, 6);
  Serial.print("Accuracy = ");
  Serial.println(accuracy);

   if (latitude < 1){ location();}
    else{ 
    sendNotification("Uw voertuig bevindt zich op de volgende coördinaten: http://www.google.com/maps/place/" + String(latitude, 6) + "," + String(longitude, 6));
    }
  }
