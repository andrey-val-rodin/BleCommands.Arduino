#include "BLECommands.h"

BLECommandsServer* BLECommandsServer::_instance = nullptr;
char TERMINATOR = '\n';

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
    if (token.length() > 0 && token[token.length() - 1] == TERMINATOR) {
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
    return write(listeningCharacteristic, token);
}

int BLECommandsServer::writeResponse(const String& response) {
    return write(responseCharacteristic, response);
}

int BLECommandsServer::write(BLECharacteristic& characteristic, const String& value) {
    if (!characteristic.subscribed()) return 0;

    const size_t MAX_CHUNK_SIZE = 200;
    const int len = value.length();
    const uint8_t* data = (const uint8_t*)value.c_str();
    const size_t TERMINATOR_SIZE = 1;
    uint8_t buf[MAX_CHUNK_SIZE + TERMINATOR_SIZE];    

    if (len < MAX_CHUNK_SIZE) {
        memcpy(buf, data, len);
        buf[len] = TERMINATOR;
        return characteristic.writeValue(buf, len + 1);
    }

    int pos = 0;
    while (true) {
        int remaining = len - pos;
        bool isLast = (remaining < MAX_CHUNK_SIZE);
        int chunkLen = isLast ? remaining : MAX_CHUNK_SIZE;

        if (!isLast && pos + chunkLen < len) {
            // UTF-8 correction: don't cut characters
            while (chunkLen > 0 && (data[pos + chunkLen] & 0xC0) == 0x80) {
                chunkLen--;
            }

            if (chunkLen <= 0) { // corrupted UTF-8 string
                // All bytes from pos to len-1 are continuation bytes (invalid UTF-8 or mid-sequence start)
                // Send at least 1 byte to avoid infinite loop
                chunkLen = 1;
            }
        }

        memcpy(buf, data + pos, chunkLen);
        pos += chunkLen;

        int writeLen;
        if (isLast) {
            buf[chunkLen] = TERMINATOR;
            writeLen = chunkLen + 1;
        } else {
            writeLen = chunkLen;
        }

        if (!characteristic.writeValue(buf, writeLen)) {
            return 0;
        }

        if (isLast) break;
    }

    return 1;
}

void BLECommandsServer::staticCommandHandler(BLEDevice central, BLECharacteristic characteristic) {
    if (_instance) {
        auto c = static_cast<BLEStringCharacteristic&>(characteristic);
        String value = c.value();
        _instance->onTokenReceived(value);
    }
}
