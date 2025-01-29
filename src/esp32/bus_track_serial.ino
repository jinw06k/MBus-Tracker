#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <string>
#include <vector>
#include <time.h>
#include <TM1637Display.h>

// ------------------------------------
// Rotary Encoder
// ------------------------------------
#define CH_A 4
#define CH_B 21
unsigned long _lastIncReadTime = micros();
unsigned long _lastDecReadTime = micros();
int _pauseLength = 25000;
int _fastIncrement = 10;
int _numRoutes = 3;
volatile int counter = 0;

void read_encoder() {
  static uint8_t cur_state = 0b11;
  static int8_t ticks = 0;
  static const int8_t enc_states[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

  cur_state <<= 2;

  if (digitalRead(CH_A)) cur_state |= 0b10; 
  if (digitalRead(CH_B)) cur_state |= 0b01;

  ticks += enc_states[( cur_state & 0x0f )];

  // clockwise turn
  if (ticks > 3) {
    int changevalue = 1;
    if ((micros() - _lastIncReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue;
    }
    _lastIncReadTime = micros();
    counter = counter + changevalue;
    // modulo to numRoutes
    counter = (counter+(_numRoutes*10))%_numRoutes;
    ticks = 0;
  }
  // counter clockwise turn
  else if (ticks < -3) {
    int changevalue = -1;
    if ((micros() - _lastDecReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue;
    }
    _lastDecReadTime = micros();
    counter = counter + changevalue;
    // modulo to numRoutes
    counter = (counter+(_numRoutes*10))%_numRoutes;
    ticks = 0;
  }
}
// ------------------------------------
// END Rotary Encoder
// ------------------------------------


// ------------------------------------
// Current Time Display
// ------------------------------------
#define CLK 12
#define DIO 14
TM1637Display display = TM1637Display(CLK, DIO);
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;
const long daylightOffset_sec = 0;

void get_current_time() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }
  else{
    Serial.println(&timeinfo, "Current Time - %H:%M:%S");
  }

  int curTime = timeinfo.tm_hour * 100 + timeinfo.tm_min;
  display.showNumberDecEx(curTime, 0b01000000);
}
// ------------------------------------
// END Current Time Display
// ------------------------------------


// ------------------------------------
// WiFi
// ------------------------------------
const char* ssid = "";
const char* password = "";

void connectWiFi(){
  WiFi.begin(ssid, password);
  Serial.println("Connecting");

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("... ");
  }
  Serial.println("");

  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}
// ------------------------------------
// END WiFi
// ------------------------------------


// ------------------------------------
// API, Bus Prediction
// ------------------------------------
const char* endpoint = "https://mbus.ltp.umich.edu/bustime/api/v3/getpredictions?key=";
const char* key = "";

struct StopData {
  String routeId;
  String dir;
  String stopId;
  String vehicleId;
  String arrivalTime;
};

// routes to predict
std::vector<std::tuple<String, String, String>> northboundFromCCTC = {
    {"BB", "NORTHBOUND", "C250"},
    {"NW", "NORTHBOUND", "C251"},
    {"CN", "NORTHBOUND", "C250"}
};

// get Json data from https GET
void get_data(const String& url, DynamicJsonDocument& doc) {
    HTTPClient http;
    http.begin(url);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);

    int httpResponseCode = http.GET();

    if (httpResponseCode == 302) {
        String redirectUrl = http.header("Location");
        Serial.println("Redirected to: " + redirectUrl);
    }

    if (httpResponseCode == 200) {
        String payload = http.getString();
        deserializeJson(doc, payload);
    }

    http.end();
    return;
}

void predict_bus(std::tuple<String, String, String> entry) {
  String routeId, dir, stopId;
  std::tie(routeId, dir, stopId) = entry;

  String url = String(endpoint) + key + "&format=json&rt=" + routeId + "&stpid=" + stopId + "&tmres=s";
  DynamicJsonDocument doc(2048);
  get_data(url, doc);

  std::vector<StopData> result;

  if (doc.containsKey("bustime-response")) {
    JsonObject response = doc["bustime-response"];
    
    if(response.containsKey("prd")){
        JsonArray predictions = response["prd"].as<JsonArray>();
        
        for (JsonObject prediction : predictions) {
          if (prediction["rtdir"].as<String>() == dir) {
            StopData stopData = {
              routeId,
              dir,
              stopId,
              prediction["vid"].as<String>(),
              prediction["prdctdn"].as<String>()
            };
            result.push_back(stopData);
          }
        }
    }
  }

  for (const auto& stopData : result) {
        Serial.println("Route ID: " + stopData.routeId);
        Serial.println("Direction: " + stopData.dir);
        Serial.println("Stop ID: " + stopData.stopId);
        Serial.println("Vehicle ID: " + stopData.vehicleId);
        Serial.println("Arrival Time: " + stopData.arrivalTime);
        Serial.println();
  }
}
// ------------------------------------
// END API, Bus Prediction
// ------------------------------------


void setup() {
  Serial.begin(115200); 

  connectWiFi();
  
  // Current Time Display
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  get_current_time();
  display.clear();
  display.setBrightness(7);

  // Configure Rotary Encoder
  pinMode(CH_A, INPUT_PULLUP);
  pinMode(CH_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CH_A), read_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(CH_B), read_encoder, CHANGE);
}

int reloadTime = 0;
#define RELOAD_INTERVAL 10000
void loop() {

  static int lastCounter = 0;
  if (counter != lastCounter) {
    Serial.println(counter);
    Serial.print("Route: ");
    Serial.println(std::get<0>(northboundFromCCTC[counter]));
    lastCounter = counter;
    reloadTime = RELOAD_INTERVAL;
  }
  
  if ((reloadTime >= RELOAD_INTERVAL) && (WiFi.status() == WL_CONNECTED)) { 
    Serial.println("###########################");
    Serial.println("Predicting bus...");
    predict_bus(northboundFromCCTC[counter]);
    Serial.println("###########################");
    reloadTime = 0;
  }

  delay(10);    
  reloadTime += 10;
}