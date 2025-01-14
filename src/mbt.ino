#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "";
const char* password = "";

const String endpoint = "http://mbus.ltp.umich.edu/bustime/api/v3/getroutes?key=";
const String key = "";

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

void setup() {
  Serial.begin(115200); 

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop() {
    if ((WiFi.status() == WL_CONNECTED)) { 
 
    HTTPClient http;
 
    http.begin(endpoint + key + "&format=json");
    int httpCode = http.GET();
 
    if (httpCode > 0) { 
 
        String payload = http.getString();

        Serial.println(payload);
      }
 
      else {
        Serial.println("Error on HTTP request");
      }
 
    http.end(); 
  }
 
  delay(30000);
}