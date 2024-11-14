#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>

const char* serverIP = "192.168.0.104";  // Replace with your server IP
WiFiClient client;
const int sensorUpPin = 4;
const int sensorDownPin = 5;
WiFiManager wifiManager;  // Initialize WiFi Manager

void setup() {
  pinMode(sensorUpPin, INPUT_PULLUP);  // Ensure GPIO0 is pulled high
  pinMode(sensorDownPin, INPUT_PULLUP);  
  
  Serial.begin(74880); // Set baud rate for serial monitor

  // Initialize WiFi Manager and setup WiFi connection
  Serial.println("Starting WiFi Manager...");

  // Uncomment the line below to reset WiFi credentials (if needed)
  // wifiManager.resetSettings();

  // Automatically connect or create an access point if no saved credentials are found
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("Failed to connect to WiFi. Restarting...");
    ESP.restart();  // If failed to connect, restart the ESP
  }

  // If we reach here, WiFi has connected successfully
  Serial.println("Connected to WiFi successfully!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  int sensorUpValue = digitalRead(sensorUpPin);
  int sensorDownValue = digitalRead(sensorDownPin);

  Serial.print("Sensor Up Value: ");
  Serial.print(sensorUpValue);
  Serial.print(" & Sensor Down Value: ");
  Serial.println(sensorDownValue);

  if (sensorUpValue == LOW && sensorDownValue == LOW) {
    turnOffPin();
  } else if (sensorUpValue == HIGH && sensorDownValue == HIGH) {
    turnOnPin();
  }
  //ESP.deepSleep(0); // deep sleep for reset
  //deep sleep makes issue while waking up on first reset.
  WiFi.forceSleepBegin(); // Initiate light sleep
  delay(3e5); // Sleep for 5 minutes
  WiFi.forceSleepWake(); // Wake up from light sleep

  delay(1000); // Short delay for stability
}

void loop() {
  // The loop function is left empty as the task is handled in setup.
}

void turnOnPin() {
  HTTPClient http;
  String url = "http://" + String(serverIP) + "/12/on";
  http.begin(client, url);
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("Pin 12 turned on");
    http.end();
  } else {
    Serial.println("Failed to turn on Pin 12");
    http.end();
    //ESP.deepSleep(10e6); // going deep sleep for x seconds before it retries.
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
  } else {
    Serial.println("Failed to turn off Pin 12");
    http.end();
    //ESP.deepSleep(10e6); // going deep sleep for x seconds before it retries.
  }
}
