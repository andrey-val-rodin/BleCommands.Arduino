#include "BLECommands.h"

BLECommandsServer server;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!server.begin("BLECommands device")) {
    Serial.println("Starting BLECommands server failed");
    while (true);
  }

  server
  .onCommand("STATUS", [](const String& command, const String& args) -> String { return "READY"; })
  .onCommand("GET", [](const String& command, const String& args) -> String {
    if (args == "MAC") {
      return BLE.address();
    } else if (args == "NAME") {
      return "BLECommands device";
    }

    return "";
  });

  Serial.println("Successfully initialized");
}

void loop() {
  server.poll();
}
