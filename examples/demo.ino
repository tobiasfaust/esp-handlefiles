#include <WiFi.h> 
#include <AsyncTCP.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <handleFiles.h>

AsyncWebServer server(80);
handleFiles* fsfiles = new handleFiles(&server);

void setup() {
    // Initialize serial communication at 9600 bits per second
    Serial.begin(115200);
    
    // Print a message to the serial monitor
    Serial.println("Hello, Arduino!");

    // Connect to Wi-Fi
    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to the WiFi network");

    server.begin();
}

void loop() {
    // Print a message to the serial monitor every second
    Serial.println("Looping...");
    
    // Wait for a second
    delay(1000);
}