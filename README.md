# BLECommands

An Arduino library on the base of ArduinoBLE for implementing text-based command/response protocols over Bluetooth Low Energy (BLE). Designed to work seamlessly with the accompanying .NET **BLECommands** client library.

## Features

- **Simple command registration** using functions or lambdas
- **Automatic token parsing** with command name and arguments extraction
- **Configurable fallback handler** for unknown/invalid commands
- **Fluent interface** for elegant code
- **Platform independent** — works with all boards supported by ArduinoBLE

For more information about this library, visit https://github.com/andrey-val-rodin/BleCommands.

## Example
```c++
#include <BLECommands.h>

BLECommandsServer server;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  server
    .onCommand("PING", [](const String& cmd, const String& args) -> String {
      return "PONG";
      })
    .onCommand("ECHO", [](const String& cmd, const String& args) -> String {
      return args.isEmpty() ? cmd : cmd + " " + args;
      });

    if (!server.begin("BLECommands Device")) {
        Serial.println("Failed to start server");
        while (true);
    }

    Serial.println("Server ready! Commands: PING, ECHO");
}

void loop() {
    server.poll();
}
```

>You can add command handlers as external functions or lambdas. In any case, remember to return a value from the function, otherwise the sketch's behavior will be unpredictable.
