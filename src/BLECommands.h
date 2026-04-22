#pragma once

#include <ArduinoBLE.h>
#include <map>
#include <functional>

#define TERMINATOR "\n"
#define SERVICE_UUID    "DB341FB3-8977-4C2D-AC6C-74540BD8B901"
#define COMMAND_UUID    "DB341FB3-8977-4C2D-AC6C-74540BD8B902"
#define RESPONSE_UUID   "DB341FB3-8977-4C2D-AC6C-74540BD8B903"
#define LISTENING_UUID  "DB341FB3-8977-4C2D-AC6C-74540BD8B904"

using CommandHandler = std::function<String(const String&, const String&)>;
using FallbackHandler = std::function<String(const String&)>;

String DefaultFallbackHandler(const String& token);

struct Command {
    String commandName;
    String arguments;
    bool isValid() const { return commandName.length() > 0; }
};

class BLECommandsServer {
public:
    BLECommandsServer();
    virtual ~BLECommandsServer();

    virtual bool begin(const char* deviceName);
    virtual void end();
    virtual void poll();
    virtual void poll(unsigned long timeout);

    FallbackHandler getFallbackHandler() const { return _fallbackHandler; }
    BLECommandsServer& setFallbackHandler(FallbackHandler handler);

    BLECommandsServer& onCommand(const String& commandName, CommandHandler handler);
    BLECommandsServer& onCommand(const char* commandName, CommandHandler handler);

    virtual void onTokenReceived(String& token);

    virtual int send(const String& token);

    BLEService service;
    BLEStringCharacteristic commandCharacteristic;
    BLEStringCharacteristic responseCharacteristic;
    BLEStringCharacteristic listeningCharacteristic;

protected:
    Command parseToken(const String& token);
    int writeResponse(const String& response);

private:
    static BLECommandsServer* _instance;

    FallbackHandler _fallbackHandler;
    std::map<String, CommandHandler> _handlers;

    static void staticCommandHandler(BLEDevice central, BLECharacteristic characteristic);
};
