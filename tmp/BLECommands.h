#pragma once

#include <ArduinoBLE.h>

#define SERVICE_UUID    "DB341FB3-8977-4C2D-AC6C-74540BD8B901"
#define COMMAND_UUID    "DB341FB3-8977-4C2D-AC6C-74540BD8B902"
#define RESPONSE_UUID   "DB341FB3-8977-4C2D-AC6C-74540BD8B903"
#define LISTENING_UUID  "DB341FB3-8977-4C2D-AC6C-74540BD8B904"

class BLECommandsServer {
public:
    BLECommandsServer();
    virtual ~BLECommandsServer();

    virtual bool begin(const char *localName);
    virtual void poll();
    virtual void poll(unsigned long timeout);

    virtual void onCommandReceived(String command);

private:
    static BLECommandsServer* _instance;

    BLEService _service;
    BLEStringCharacteristic _commandCharacteristic;
    BLEStringCharacteristic _responseCharacteristic;
    BLEStringCharacteristic _listeningCharacteristic;

    static void staticCommandHandler(BLEDevice central, BLECharacteristic characteristic);
};
