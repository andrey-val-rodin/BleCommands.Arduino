/*
  Messaging Example - Demonstrating messages sent to the client
 
  Commands:
    START  -> Starts messaging, returns "OK" if messaging is not in progress; "ERR"otherwise
    STOP   -> Stops messaging, returns "OK" if messaging is in progress; "ERR"otherwise

  See corresponded .NET sample: https://github.com/andrey-val-rodin/BleCommands/tree/main/Source/Samples/Wpf
*/

#include <BLECommands.h>

BLECommandsServer server;
bool progress;
int currentPos = 0;

void stopMessaging() {
  progress = false;
  currentPos = 0;
  server.send("END"); // notify server that messaging is finished
}

void setup() {
  server
    .onCommand("START", [](const String& cmd, const String& args) -> String {
        if (progress) {
          return "ERR";
        } else {
          progress = true;
          return "OK";
        }
      })
    .onCommand("STOP", [](const String& cmd, const String& args) -> String {
        if (!progress) {
          return "ERR";
        } else {
          stopMessaging();
          return "OK";
        }
      });

    if (!server.begin("BLECommands Messaging Example")) {
        Serial.println("Failed to start server");
        while (true);
    }
}

unsigned long timer = millis();
void loop() {
  server.poll();
  
  if (progress && millis() - timer >= 500) {
    currentPos++;
    if (currentPos > 100) {
      stopMessaging();
    } else {
      server.send("POS " + String(currentPos));
      timer = millis();
    }
  }
}
