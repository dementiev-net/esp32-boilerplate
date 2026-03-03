#include "ble.h"

#include <Arduino.h>
#include <string>

#include "../../config.h"
#if FEATURE_BLE
#include <NimBLEDevice.h>
#include "../board/board_profile.h"

static bool initialized = false;
static bool clientConnected = false;
static NimBLECharacteristic* batteryCharacteristic = nullptr;
static NimBLECharacteristic* uptimeCharacteristic = nullptr;
static NimBLECharacteristic* versionCharacteristic = nullptr;

static int latestBatteryPercent = -1;
static unsigned long latestUptimeSeconds = 0;
static bool telemetryDirty = true;
static unsigned long lastTelemetryPublishMs = 0;

class RuntimeBleServerCallbacks : public NimBLEServerCallbacks {
public:
    void onConnect(NimBLEServer* server) override {
        (void)server;
        clientConnected = true;
        Serial.println("[BLE] Client connected");
    }

    void onDisconnect(NimBLEServer* server) override {
        (void)server;
        clientConnected = false;
        Serial.println("[BLE] Client disconnected");
        NimBLEDevice::startAdvertising();
        Serial.println("[BLE] Advertising restarted");
    }
};

static String buildBleDeviceName() {
    const BoardProfile& board = boardGetProfile();
    const char* suffix = board.id == BoardId::TDisplayS3 ? "tdisplay-s3" : "tqt-pro";
    String name = BLE_DEVICE_NAME_PREFIX;
    name += "-";
    name += suffix;
    return name;
}

static void publishTelemetry() {
    if (!batteryCharacteristic || !uptimeCharacteristic || !versionCharacteristic) {
        return;
    }

    char batteryText[8];
    if (latestBatteryPercent < 0) {
        snprintf(batteryText, sizeof(batteryText), "--");
    } else {
        snprintf(batteryText, sizeof(batteryText), "%d%%", latestBatteryPercent);
    }

    char uptimeText[16];
    snprintf(uptimeText, sizeof(uptimeText), "%lu", latestUptimeSeconds);

    batteryCharacteristic->setValue(std::string(batteryText));
    uptimeCharacteristic->setValue(std::string(uptimeText));
    versionCharacteristic->setValue(std::string(APP_VERSION));

    if (clientConnected) {
        batteryCharacteristic->notify();
        uptimeCharacteristic->notify();
    }
}

void bleInit() {
    if (initialized) {
        return;
    }

    const String deviceName = buildBleDeviceName();
    NimBLEDevice::init(deviceName.c_str());

    NimBLEServer* server = NimBLEDevice::createServer();
    server->setCallbacks(new RuntimeBleServerCallbacks());

    NimBLEService* service = server->createService(BLE_SERVICE_UUID);
    batteryCharacteristic = service->createCharacteristic(
        BLE_CHAR_BATTERY_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    uptimeCharacteristic = service->createCharacteristic(
        BLE_CHAR_UPTIME_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    versionCharacteristic = service->createCharacteristic(
        BLE_CHAR_VERSION_UUID,
        NIMBLE_PROPERTY::READ
    );
    service->start();

    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
    advertising->addServiceUUID(BLE_SERVICE_UUID);
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06);
    advertising->setMaxPreferred(0x12);

    NimBLEDevice::startAdvertising();
    initialized = true;
    telemetryDirty = true;

    Serial.printf("[BLE] Advertising started: %s\n", deviceName.c_str());
}

void bleLoop() {
    if (!initialized) {
        return;
    }

    if (!telemetryDirty) {
        return;
    }

    const unsigned long nowMs = millis();
    if (nowMs - lastTelemetryPublishMs < BLE_TELEMETRY_UPDATE_MS) {
        return;
    }

    publishTelemetry();
    telemetryDirty = false;
    lastTelemetryPublishMs = nowMs;
}

void bleSetTelemetry(int batteryPercent, unsigned long uptimeSeconds) {
    int normalizedBatteryPercent = batteryPercent;
    if (normalizedBatteryPercent < 0) {
        normalizedBatteryPercent = -1;
    } else if (normalizedBatteryPercent > 100) {
        normalizedBatteryPercent = 100;
    }

    if (normalizedBatteryPercent != latestBatteryPercent) {
        latestBatteryPercent = normalizedBatteryPercent;
        telemetryDirty = true;
    }

    if (uptimeSeconds != latestUptimeSeconds) {
        latestUptimeSeconds = uptimeSeconds;
        telemetryDirty = true;
    }
}

bool bleIsConnected() {
    return clientConnected;
}

bool bleIsReady() {
    return initialized;
}
#else

void bleInit() {}

void bleLoop() {}

void bleSetTelemetry(int batteryPercent, unsigned long uptimeSeconds) {
    (void)batteryPercent;
    (void)uptimeSeconds;
}

bool bleIsConnected() {
    return false;
}

bool bleIsReady() {
    return false;
}
#endif
