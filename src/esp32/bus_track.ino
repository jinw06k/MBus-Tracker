#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <string>
#include <vector>
#include <time.h>
#include <utility> 
#include <TM1637Display.h>

// ------------------------------------
// Rotary Encoder
// ------------------------------------
#define CH_A 22
#define CH_B 23
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
// 7 Segment Display
// ------------------------------------
#define dataPin 4
#define clockPin 19
#define latchPin 21
byte num[] {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x00};
bool busAvailable = false;

void displayNumber(std::pair<int,int> arrivalTimes) {

  digitalWrite(latchPin, LOW);
  shiftNum(arrivalTimes.second);
  shiftNum(arrivalTimes.first);
  digitalWrite(latchPin, HIGH);
}

void shiftNum(int number){
  if(number == 1000){
    Serial.println("@7seg -- no bus available");
    shiftOut(dataPin, clockPin, MSBFIRST, 64);
    shiftOut(dataPin, clockPin, MSBFIRST, 64);
  }
  else{
    Serial.print("@7seg -- Earliest ETA: ");
    Serial.println(number);

    int tens = number / 10;
    int ones = number % 10;

    if(tens == 0) tens = 10;
    
    shiftOut(dataPin, clockPin, MSBFIRST, num[ones]);
    shiftOut(dataPin, clockPin, MSBFIRST, num[tens]);
  }
}

// ------------------------------------
// END 7 Segment Display
// ------------------------------------

// ------------------------------------
// RGB LED
// ------------------------------------
#define RED_LED 14
#define GREEN_LED 12
#define BLUE_LED 13
// BB green, NW red, CN purple
int red[] = {255, 30, 100};
int green[] = {30, 255, 255};
int blue[] = {255, 255, 100};
void setLED(int route) {
  Serial.print("@RGB -- Setting LED light for route #");
  Serial.println(route);
  Serial.println("@RGB -- BB = 0, NW = 1, CN = 2");
  analogWrite(RED_LED, red[route]);
  analogWrite(GREEN_LED, green[route]);
  analogWrite(BLUE_LED, blue[route]);
}
// ------------------------------------
// END RGB LED
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

  Serial.print("@WiFi -- Connected to WiFi network with IP Address: ");
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

std::pair<int,int> predict_bus(std::tuple<String, String, String> entry) {
  String routeId, dir, stopId;
  std::tie(routeId, dir, stopId) = entry;

  String url = String(endpoint) + key + "&format=json&rt=" + routeId + "&stpid=" + stopId + "&tmres=s";
  DynamicJsonDocument doc(2048);
  get_data(url, doc);

  std::vector<StopData> result;

  std::pair<int,int> earliestArrival = {1000,1000};

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

  if(result.size() >= 1) {
    earliestArrival.first = string_to_int(result[0].arrivalTime);
  }
  if(result.size() >= 2) {
    earliestArrival.second = string_to_int(result[1].arrivalTime);
  }

  Serial.println("@predict bus -- ");
  for (const auto& stopData : result) {
        Serial.println("Route ID: " + stopData.routeId);
        Serial.println("Direction: " + stopData.dir);
        Serial.println("Stop ID: " + stopData.stopId);
        Serial.println("Vehicle ID: " + stopData.vehicleId);
        Serial.println("Arrival Time: " + stopData.arrivalTime);
        Serial.println();
  }

  return earliestArrival;
}

int string_to_int(String str) {
  int tmp = 0;
  if(str == "DUE") {
    tmp = 0;
  } else {
    tmp = str.toInt();
  }
  return tmp;
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

  // 7 Segment Display
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  // RGB LED
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
}

int reloadTime = 0;
#define RELOAD_INTERVAL 10000

std::pair<int,int> eta = {1000,1000};

void loop() {

  static int lastCounter = 0;
  if (counter != lastCounter) {
    Serial.print("Route: ");
    Serial.println(std::get<0>(northboundFromCCTC[counter]));
    lastCounter = counter;
    reloadTime = RELOAD_INTERVAL;
    setLED(counter);
  }
  
  if ((reloadTime >= RELOAD_INTERVAL) && (WiFi.status() == WL_CONNECTED)) { 
    Serial.println("###########################");
    Serial.println("Predicting bus...");
    eta = predict_bus(northboundFromCCTC[counter]);
    Serial.println("###########################");
    displayNumber(eta);
    reloadTime = 0;
  }

  delay(10);    
  reloadTime += 10;
}
