#include "BLECommands.h"

BLECommandsServer* BLECommandsServer::_instance = nullptr;

BLECommandsServer::BLECommandsServer()
    : _service(SERVICE_UUID),
      _commandCharacteristic(COMMAND_UUID, BLEWrite | BLEWriteWithoutResponse, 512),
      _responseCharacteristic(RESPONSE_UUID, BLENotify | BLEIndicate, 512),
      _listeningCharacteristic(LISTENING_UUID, BLENotify | BLEIndicate, 512)
{
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

    _service.addCharacteristic(_commandCharacteristic);
    _service.addCharacteristic(_responseCharacteristic);
    _service.addCharacteristic(_listeningCharacteristic);
    BLE.addService(_service);

    _commandCharacteristic.setEventHandler(BLEWritten, staticCommandHandler);

    // Start advertising
    return BLE.advertise();
}

void BLECommandsServer::end() { BLE.end(); }
void BLECommandsServer::poll() { BLE.poll(); }
void BLECommandsServer::poll(unsigned long timeout) { BLE.poll(timeout); }

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
    
    auto command = parseToken(token);
    if (command.isValid()) {
        processCommand(command);
    } else {
        writeResponse("INVALID");
    }
}

void BLECommandsServer::processCommand(Command& command) {
    auto it = _handlers.find(command.commandName);
    if (it != _handlers.end()) {
        writeResponse(it->second(command.commandName, command.arguments));
    } else {
        writeResponse("UNKNOWN");
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

void BLECommandsServer::send(const String& token) {
    if (isTokenValid(token)) {
        _listeningCharacteristic.writeValue(token + TERMINATOR);
    }
}

void BLECommandsServer::writeResponse(const String& response) {
    if (isTokenValid(response)) {
        _responseCharacteristic.writeValue(response + TERMINATOR);
    }
}

bool BLECommandsServer::isTokenValid(const String& token) {
    return token.length() <= MAX_TOKEN_SIZE;
}

void BLECommandsServer::staticCommandHandler(BLEDevice central, BLECharacteristic characteristic) {
    if (_instance) {
        auto c = static_cast<BLEStringCharacteristic&>(characteristic);
        String value = c.value();
        _instance->onTokenReceived(value);
    }
}
