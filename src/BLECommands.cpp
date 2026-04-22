#include "BLECommands.h"

BLECommandsServer* BLECommandsServer::_instance = nullptr;

String DefaultFallbackHandler(const String& token) {
    return token.length() == 0 ? "INVALID" : "UNKNOWN";
}

BLECommandsServer::BLECommandsServer()
    : service(SERVICE_UUID),
      commandCharacteristic(COMMAND_UUID, BLEWrite | BLEWriteWithoutResponse, 512),
      responseCharacteristic(RESPONSE_UUID, BLENotify | BLEIndicate, 512),
      listeningCharacteristic(LISTENING_UUID, BLENotify | BLEIndicate, 512)
{
    _fallbackHandler = DefaultFallbackHandler;
}

BLECommandsServer::~BLECommandsServer() {
    if (_instance == this) {
        _instance = nullptr;
        BLE.end();
    }
}

bool BLECommandsServer::begin(const char* deviceName)
{
    if (_instance != nullptr) {
        // Only one instance allowed
        return false;
    }

    _instance = this;

    if (!BLE.begin()) return false;
    if (!BLE.setLocalName(deviceName)) return false;
    if (!BLE.setAdvertisedService(SERVICE_UUID)) return false;

    service.addCharacteristic(commandCharacteristic);
    service.addCharacteristic(responseCharacteristic);
    service.addCharacteristic(listeningCharacteristic);
    BLE.addService(service);

    commandCharacteristic.setEventHandler(BLEWritten, staticCommandHandler);

    // Start advertising
    return BLE.advertise();
}

void BLECommandsServer::end() { BLE.end(); }
void BLECommandsServer::poll() { BLE.poll(); }
void BLECommandsServer::poll(unsigned long timeout) { BLE.poll(timeout); }

BLECommandsServer& BLECommandsServer::setFallbackHandler(FallbackHandler handler) {
    if (handler) {
        _fallbackHandler = handler;
    }
    return *this;
}

BLECommandsServer& BLECommandsServer::onCommand(const String& commandName, CommandHandler handler) {
    _handlers[commandName] = handler;
    return *this;
}

BLECommandsServer& BLECommandsServer::onCommand(const char* commandName, CommandHandler handler) {
    return onCommand(String(commandName), handler);
}

void BLECommandsServer::onTokenReceived(String& token) {
    if (token.length() > 0 && token.endsWith(TERMINATOR)) {
        token.remove(token.length() - 1);
    }

    if (token.length() == 0) {
        writeResponse(_fallbackHandler(token));
        return;
    }
    
    auto command = parseToken(token);
    if (command.isValid()) {
        auto it = _handlers.find(command.commandName);
        if (it != _handlers.end()) {
            writeResponse(it->second(command.commandName, command.arguments));
        } else {
            writeResponse(_fallbackHandler(token));
        }
    } else {
        writeResponse(_fallbackHandler(token));
    }
}

Command BLECommandsServer::parseToken(const String& token) {
    Command result;
    if (token.length() == 0) return result;
    
    int spaceIndex = token.indexOf(' ');
    if (spaceIndex < 0) {
        result.commandName = token;
        result.arguments = "";
    } else {
        result.commandName = token.substring(0, spaceIndex);
        result.arguments = token.substring(spaceIndex + 1);
    }
    
    return result;
}

int BLECommandsServer::send(const String& token) {
    return listeningCharacteristic.writeValue(token + TERMINATOR);
}

int BLECommandsServer::writeResponse(const String& response) {
    return responseCharacteristic.writeValue(response + TERMINATOR);
}

void BLECommandsServer::staticCommandHandler(BLEDevice central, BLECharacteristic characteristic) {
    if (_instance) {
        auto c = static_cast<BLEStringCharacteristic&>(characteristic);
        String value = c.value();
        _instance->onTokenReceived(value);
    }
}
