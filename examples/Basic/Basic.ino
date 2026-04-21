/*
  Basic Example - Minimal BLECommands server setup

  This example demonstrates the simplest usage of the library:
  - Register command handlers using regular functions
  - Start the BLE server
  - Poll for incoming commands in the main loop
 
  Commands:
    PING  -> Returns "PONG"
    ECHO  -> Returns the command itself (e.g., "ECHO Hello" -> "ECHO Hello")
*/

#include <BLECommands.h>

BLECommandsServer server;

String handlePing(const String&, const String&) {
  return "PONG";
}

String handleEcho(const String& command, const String& args) {
  return args.isEmpty() ? command : command + " " + args;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!server.begin("BLECommands device")) {
    Serial.println("Starting BLECommands server failed");
    while (true);
  }

  server
    .onCommand("PING", handlePing)
    .onCommand("ECHO", handleEcho);
}

void loop() {
  server.poll();
}
