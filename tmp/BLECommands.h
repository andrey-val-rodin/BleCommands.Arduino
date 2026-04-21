#pragma once

#include <ArduinoBLE.h>
#include <map>

#define MAX_TOKEN_SIZE 200
#define TERMINATOR "\n"
#define SERVICE_UUID    "DB341FB3-8977-4C2D-AC6C-74540BD8B901"
#define COMMAND_UUID    "DB341FB3-8977-4C2D-AC6C-74540BD8B902"
#define RESPONSE_UUID   "DB341FB3-8977-4C2D-AC6C-74540BD8B903"
#define LISTENING_UUID  "DB341FB3-8977-4C2D-AC6C-74540BD8B904"

using CommandHandler = std::function<String(const String&, const String&)>;

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

    BLECommandsServer& onCommand(const String& commandName, CommandHandler handler);
    BLECommandsServer& onCommand(const char* commandName, CommandHandler handler);

    virtual void onTokenReceived(String& token);

    virtual void send(const String& token);

protected:
    Command parseToken(const String& token);
    void processCommand(Command& command);
    void writeResponse(const String& response);
    bool isTokenValid(const String& token);

private:
    static BLECommandsServer* _instance;

    BLEService _service;
    BLEStringCharacteristic _commandCharacteristic;
    BLEStringCharacteristic _responseCharacteristic;
    BLEStringCharacteristic _listeningCharacteristic;
    std::map<String, CommandHandler> _handlers;

    static void staticCommandHandler(BLEDevice central, BLECharacteristic characteristic);
};
