#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // Include WiFiManager library
#include <ArduinoJson.h>
#include "index.h"

// Create an instance of the web server
ESP8266WebServer server(80);

// Set static IP (optional)
IPAddress local_IP(192, 168, 0, 104);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 0, 0);

// Timing variables
unsigned long pumpOnTime = 0; // Time when the pump was turned on
unsigned long pumpOffTime = 0; // Time when the pump was turned off

// Global variables to manage pump timing
unsigned long pumpDuration = 0; // Duration to keep the pump on
unsigned long pumpStartTime = 0; // Time when the pump was turned on
bool pumpRunning = false; // Flag to check if the pump is currently running

// Define the GPIO pin for output
const int output = 12;
const int wifi_led = 02;
const int pump_led = 14;
const int analogInPin = A0;  // ESP8266 Analog Pin ADC0 = A0
const int buttonPin = 0;      // GPIO 0 for the momentary push button
String outputState = "off";

// Define variables for external switch
int acSwitchPin = 13; // Pin connected to detect external switch
int switchState = LOW;
int prevState = LOW;
long zerotime = 0;
long onetime = 0;

// Function prototypes
void handleRoot();
void handlePumpOn();
void handlePumpOff();
void handleGetState();
void handlePumpOnForDuration(); // New function prototype
int checkExternalSwitch();

void setup() {
    Serial.begin(115200);

    // Initialize GPIO pins
    pinMode(output, OUTPUT);
    pinMode(wifi_led, OUTPUT);
    pinMode(pump_led, OUTPUT);
    pinMode(acSwitchPin, INPUT);
    pinMode(analogInPin, INPUT);
    pinMode(buttonPin, INPUT_PULLUP);  // Use internal pull-up resistor for the button

    digitalWrite(output, LOW); // Turns off pump relay
    digitalWrite(wifi_led, HIGH);  // Turns off wifi indicator LED
    digitalWrite(pump_led, HIGH);  // Turns off pump indicator LED

    // Use WiFiManager to manage Wi-Fi credentials
    WiFiManager wifiManager;

    // Attempt to auto-connect, if it fails, it will create a portal for users to input credentials
    if (!wifiManager.autoConnect("PumpAutoServer", "1234")) {
        Serial.println("Failed to connect and hit timeout");
        ESP.reset(); // Restart ESP if connection fails
        delay(500);
        digitalWrite(wifi_led, LOW);  // Turns on wifi indicator LED
        delay(500);
        digitalWrite(wifi_led, HIGH);  // Turns off wifi indicator LED
    }

    Serial.println("Connected to Wi-Fi!");
    digitalWrite(wifi_led, LOW);  // Turns on wifi indicator LED

    // Optional: Configure static IP address if necessary
    if (!WiFi.config(local_IP, gateway, subnet)) {
        Serial.println("Failed to configure static IP");
    }

    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Start the web server and define request handlers
    server.on("/", HTTP_GET, handleRoot);
    server.on("/12/on", HTTP_GET, handlePumpOn);
    server.on("/12/off", HTTP_GET, handlePumpOff);
    server.on("/state", HTTP_GET, handleGetState);
    server.on("/pump/on", HTTP_GET, handlePumpOnForDuration); // New handler for duration

    server.begin();
}

void loop() {
    server.handleClient(); // Handle incoming client requests

    // Check for external switch state
    int currentSwitchState = checkExternalSwitch();
    if (prevState != currentSwitchState) {
        prevState = currentSwitchState; // Update previous state
        if (currentSwitchState == HIGH) {
            pumpOnTime = millis(); // Record the time the pump was turned on
            Serial.println("Pump turned ON");
        } else {
            pumpOffTime = millis(); // Record the time the pump was turned off
            Serial.println("Pump turned OFF");
        }
    }

    // Check if the pump is running and manage timing
    if (pumpRunning) {
        unsigned long currentTime = millis();
        if (currentTime - pumpStartTime >= pumpDuration) {
            // Time to turn off the pump
            digitalWrite(output, LOW); // Turn off the pump
            digitalWrite(pump_led, HIGH);  // Turns off pump indicator LED
            outputState = "off";
            pumpRunning = false; // Reset the pump running flag
            Serial.println("Pump turned OFF after " + String(pumpDuration / 1000) + " seconds");
        }
    }

    // Check for button press to turn on the pump
    if (digitalRead(buttonPin) == LOW) { // Button is pressed (active low)
        handlePumpOn(); // Call the function to turn on the pump
        delay(500); // Debounce delay
    }
}

void handleRoot() {
    server.send(200, "text/html", webpage); // Serve the HTML content
}

void handlePumpOn() {
    if (checkExternalSwitch() == LOW) { // Only turn on the pump if the external switch is off
        digitalWrite(output, HIGH);
        digitalWrite(pump_led, LOW);  // Turns on pump indicator LED
        outputState = "on";
        Serial.println("GPIO 12 on");
        server.send(200, "text/plain", "Pump turned ON");
    } else {
        server.send(200, "text/plain", "Pump is already on.");
    }
}

void handlePumpOff() {
    digitalWrite(output, LOW);
    digitalWrite(pump_led, HIGH);  // Turns off pump indicator LED
    outputState = "off";
    Serial.println("GPIO 12 off");
    server.send(200, "text/plain", "Pump turned OFF");
}

void handleGetState() {
    StaticJsonDocument<200> doc;
    doc["pump"] = outputState;
    doc["switch"] = (checkExternalSwitch() == HIGH) ? "on" : "off"; // Send external switch state

    // Calculate duration based on pump state
    if (outputState == "on") {
        doc["duration"] = getDuration(pumpOnTime, millis());
    } else {
        doc["duration"] = getDuration(pumpOffTime, millis());
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response); // Send the state as JSON
}

// New function to handle turning on the pump for a specified duration
void handlePumpOnForDuration() {
    String durationStr = server.arg("duration"); // Get the duration from the request
    int duration = durationStr.toInt(); // Convert to integer

    if (duration > 0) { // Ensure valid duration
        if (checkExternalSwitch() == LOW) { // Only turn on the pump if the external switch is off
            digitalWrite(output, HIGH);
            digitalWrite(pump_led, LOW);  // Turns on pump indicator LED
            outputState = "on";
            pumpRunning = true; // Set pump running flag
            pumpDuration = duration * 1000; // Convert duration to milliseconds
            pumpStartTime = millis(); // Record the time the pump was turned on
            Serial.println("Pump turned ON for " + String(duration) + " seconds");
            server.send(200, "text/plain", "Pump turned ON for " + String(duration) + " seconds");
        } else {
            server.send(200, "text/plain", "Pump is already on.");
        }
    } else {
        server.send(400, "text/plain", "Invalid duration");
    }
}


int checkExternalSwitch() {
    long windowTime = 15;  // Time window to sample (in milliseconds)
    int pinstate = digitalRead(acSwitchPin);
    if (pinstate == LOW) {
        switchState = HIGH;
        zerotime = millis();
    } else {
        onetime = millis();
        if ((onetime - zerotime) > windowTime) {
            switchState = LOW;
        }
    }
    return switchState;
}

// Function to calculate duration
String getDuration(unsigned long startTime, unsigned long currentTime) {
    unsigned long elapsedTime = (currentTime - startTime) / 1000; // Convert to seconds
    unsigned long days = elapsedTime / 86400; // Seconds in a day
    unsigned long hours = (elapsedTime % 86400) / 3600; // Seconds in an hour
    unsigned long minutes = (elapsedTime % 3600) / 60; // Seconds in a minute
    unsigned long seconds = elapsedTime % 60; // Remaining seconds

    String duration = "";
    if (days > 0) {
        duration += String(days) + " day" + (days > 1 ? "s" : "") + " ";
    }
    if (hours > 0) {
        duration += String(hours) + " hour" + (hours > 1 ? "s" : "") + " ";
    }
    if (minutes > 0) {
        duration += String(minutes) + " minute" + (minutes > 1 ? "s" : "") + " ";
    }
    if (seconds > 0 || duration.isEmpty()) { // Show seconds if there's no higher time unit
        duration += String(seconds) + " second" + (seconds > 1 ? "s" : "") + " ";
    }  

    duration.trim(); // Remove trailing spaces
    Serial.println(duration);
    return duration; // Return the modified string
}
