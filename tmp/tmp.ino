#include "BLECommands.h"

BLECommandsServer server;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!server.begin("BLECommands device")) {
    Serial.println("Starting BLECommands server failed");
    while (true);
  }

  Serial.println("Successfully initialized");
}

void loop() {
  server.poll();
}
