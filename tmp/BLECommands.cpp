#include "BLECommands.h"

BLECommandsServer* BLECommandsServer::_instance = nullptr;

BLECommandsServer::BLECommandsServer() : _service(SERVICE_UUID),
    _commandCharacteristic(COMMAND_UUID, BLEWrite | BLEWriteWithoutResponse, 512 ),
    _responseCharacteristic(RESPONSE_UUID, BLENotify | BLEIndicate, 512 ),
    _listeningCharacteristic(LISTENING_UUID, BLENotify | BLEIndicate, 512 )
{
}

BLECommandsServer::~BLECommandsServer()
{
    if (_instance == this) {
        _instance = nullptr;
    }
}

bool BLECommandsServer::begin(const char *localName)
{
    if (_instance != nullptr) {
        // Only one instance allowed
        return false;
    }

    _instance = this;

    if (!BLE.begin()) {
        // Starting Bluetooth® Low Energy module failed
        return false;
    }
    
    if (!BLE.setLocalName(localName)) {
        return false;
    }
    if (!BLE.setAdvertisedService(SERVICE_UUID)) {
        return false;
    }

    _service.addCharacteristic(_commandCharacteristic);
    _service.addCharacteristic(_responseCharacteristic);
    _service.addCharacteristic(_listeningCharacteristic);

    BLE.addService(_service);

    _commandCharacteristic.setEventHandler(BLEWritten, staticCommandHandler);

    // Start advertising
    return BLE.advertise();
}

void BLECommandsServer::poll() {
    BLE.poll();
}

void BLECommandsServer::poll(unsigned long timeout) {
    BLE.poll(timeout);
}

void BLECommandsServer::onCommandReceived(String command) {
    Serial.println("Received: " + command);
    _responseCharacteristic.writeValue("Received: " + command);
}

void BLECommandsServer::staticCommandHandler(BLEDevice central, BLECharacteristic characteristic) {
    if (_instance) {
        _instance->onCommandReceived(((BLEStringCharacteristic&)characteristic).value());
    }
}
