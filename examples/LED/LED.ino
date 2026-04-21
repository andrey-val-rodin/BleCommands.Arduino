/*
  LED Control Example - Demonstrating lambda handlers and fallback handler

  This example shows:
  - Using lambda functions as command handlers (C++ 11 feature)
  - Custom fallback handler for unknown commands
  - Controlling a physical LED via BLE commands
  - Maintaining state (isOn) across commands
 
  Commands:
    ON     -> Turns LED on, returns "OK"
    OFF    -> Turns LED off, returns "OK"
    TOGGLE -> Toggles LED state, returns "OK"
    STATE  -> Returns current state: "ON" or "OFF"
    (any other command) -> Returns "UNKNOWN: '<command>'"
*/

#include "BLECommands.h"

#ifndef LED_BUILTIN
    #define LED_BUILTIN 2  // Default for ESP32 dev boards without built-in LED
#endif

const int ledPin = LED_BUILTIN;
BLECommandsServer server;
bool isOn = false;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, isOn ? HIGH : LOW);

  server
    .setFallbackHandler([](const String& token) -> String {
        return "UNKNOWN: '" + token + "'";
      })
    .onCommand("ON", [](const String& cmd, const String& args) -> String {
        isOn = true;
        digitalWrite(ledPin, HIGH);
        return "OK";
      })
    .onCommand("OFF", [](const String& cmd, const String& args) -> String {
        isOn = false;
        digitalWrite(ledPin, LOW);
        return "OK";
      })
    .onCommand("TOGGLE", [](const String& cmd, const String& args) -> String {
        isOn = !isOn;
        digitalWrite(ledPin, isOn ? HIGH : LOW);
        return "OK";
      })
    .onCommand("STATE", [](const String& cmd, const String& args) -> String {
        return isOn ? "ON" : "OFF";
      });

    if (!server.begin("BLECommands LED Example")) {
        Serial.println("Failed to start server");
        while (true);
    }

    Serial.println("Server ready! Commands: ON, OFF, TOGGLE, STATE");
}

void loop() {
    server.poll();
}
