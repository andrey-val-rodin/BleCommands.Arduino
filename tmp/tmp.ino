#include "BLECommands.h"

BLECommandsServer server;

String handleStatus(const String&, const String&) {
    return "READY";
}

String handleGet(const String&, const String& args) {
    if (args == "MAC") return BLE.address();
    if (args == "NAME") return "BLECommands device";
    return "";
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!server.begin("BLECommands device")) {
    Serial.println("Starting BLECommands server failed");
    while (true);
  }

  server
    .onCommand("ECHO", [](const String& command, const String& args) -> String {
      return args.isEmpty() ? command : command + " " + args;
    })
    .onCommand("PING", [](const String& command, const String& args) -> String { return "PONG"; })
    .onCommand("STATUS", handleStatus)
    .onCommand("GET", handleGet);

  Serial.println("Successfully initialized");
}

void loop() {
  server.poll();
}
