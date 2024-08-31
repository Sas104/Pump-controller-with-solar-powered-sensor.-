#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "index.h" // Include the header file with HTML content

// Replace with your network credentials
const char* ssid = "Mofidul";
const char* password = "Mofidulbd";

// Create an instance of the web server
ESP8266WebServer server(80);

// Set static IP
// Set your Static IP address
IPAddress local_IP(192, 168, 0, 104);
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 0, 0);

// Define the GPIO pin for output
const int output = 12;
const int wifi_led = 02;
const int pump_led= 14;
String outputState = "off";

// Function prototypes
void handleRoot();
void handlePumpOn();
void handlePumpOff();
void handleGetState();

void setup() {
    Serial.begin(115200);

    // Initialize the output variable as an output
    pinMode(output, OUTPUT);
    pinMode(wifi_led, OUTPUT);
    pinMode(pump_led, OUTPUT);
    digitalWrite(output, LOW);
    digitalWrite(wifi_led, HIGH);
    digitalWrite(pump_led, HIGH);

    // Configure static IP address
    if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
    }

    // Connect to Wi-Fi network
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        digitalWrite(wifi_led, HIGH);
        delay(500);
        Serial.print(".");
        digitalWrite(wifi_led, LOW);
    }
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Start the server and define request handlers
    server.on("/", HTTP_GET, handleRoot);
    server.on("/12/on", HTTP_GET, handlePumpOn);
    server.on("/12/off", HTTP_GET, handlePumpOff);
    server.on("/state", HTTP_GET, handleGetState);

    server.begin();
}

void loop() {
    server.handleClient(); // Handle incoming client requests
}

void handleRoot() {
    server.send(200, "text/html", webpage); // Serve the HTML content
}

void handlePumpOn() {
    digitalWrite(output, HIGH);
    digitalWrite(pump_led, LOW);  //turns on
    outputState = "on";
    Serial.println("GPIO 12 on");
    server.send(200, "text/plain", "Pump turned ON");
}

void handlePumpOff() {
    digitalWrite(output, LOW);
    digitalWrite(pump_led, HIGH);  //turns off
    outputState = "off";
    Serial.println("GPIO 12 off");
    server.send(200, "text/plain", "Pump turned OFF");
}

void handleGetState() {
    server.send(200, "text/plain", outputState); // Send the current state of the pump
}
