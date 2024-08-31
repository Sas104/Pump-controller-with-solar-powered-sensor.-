#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Replace with your network credentials
const char* ssid = "abcde";
const char* password = "*****";

const char* serverIP = "192.168.0.104";

WiFiClient client;

const int led=2;
const int sensorUpPin = 4;
const int sensorDownPin = 5;

void setup() {
  pinMode(sensorUpPin, INPUT_PULLUP);
  pinMode(sensorDownPin, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  digitalWrite(led,HIGH);
  Serial.begin(115200);
  int i=0;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(led, LOW);
    delay(500);
    Serial.println("Connecting to WiFi...");
    digitalWrite(led, HIGH);
    delay(500);
    i++;
    if(i>10){
      ESP.deepSleep(5*6e7); // deep sleep for 5 minutes before it retries to connect.
    }
  }
  Serial.println("Connected to WiFi");
  digitalWrite(led, LOW);
  int sensorUpValue = digitalRead(sensorUpPin);
  int sensorDownValue = digitalRead(sensorDownPin);
  Serial.print(sensorUpValue);
  Serial.print("  s  ");
  Serial.println(sensorDownValue);
  if (sensorUpValue == LOW && sensorDownValue == LOW ) {
    turnOffPin();
  } else if (sensorUpValue == HIGH && sensorDownValue == HIGH) {
    turnOnPin();
  }
  delay(50);
  //Serial.print("going to deep sleep");
  //ESP.deepSleep(0); //deep sleep until restart
}

void loop() {
  
}

void turnOnPin() {
  HTTPClient http;
  String url = "http://" + String(serverIP) + "/12/on";
  http.begin(client, url);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("Pin 12 turned on");
    http.end();
    WiFi. mode(WIFI_OFF); 
    digitalWrite(led,HIGH);
  } else {
    Serial.println("Failed to turn on Pin 12");
    http.end();
    ESP.deepSleep(10e6); //going deep sleep for x seconds before it retries.
  }
}

void turnOffPin() {
  HTTPClient http;
  String url = "http://" + String(serverIP) + "/12/off";
  http.begin(client, url);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("Pin 12 turned off");
    http.end();
    WiFi.mode(WIFI_OFF); 
    digitalWrite(led,HIGH);
  } else {
    Serial.println("Failed to turn off Pin 12");
    http.end();
    ESP.deepSleep(10e6);//going deep sleep for x seconds before it retries.
  }
}
