#include <Arduino.h>
#include "config.h"
#include "modules/display/display.h"
#include "modules/wifi/wifi.h"
#include "modules/storage/storage.h"
#include "modules/buttons/buttons.h"

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println("Booting...");

    storageInit();
    displayInit();
    buttonsInit();

    buttonsOnTopClick([]() {
        Serial.println("[Button] Top click");
    });

    buttonsOnTopLongPress([]() {
        Serial.println("[Button] Top long press - resetting WiFi...");
        displayClear(TFT_BLACK);
        displayPrint(10, 10, "Resetting WiFi...", TFT_RED, 2);
        wifiReset();
    });

    buttonsOnBottomClick([]() {
        Serial.println("[Button] Bottom click");
    });

    buttonsOnBottomLongPress([]() {
        Serial.println("[Button] Bottom long press");
    });

    displayClear(TFT_BLACK);
#ifdef BOARD_TDISPLAY_S3
    displayPrint(10, 10, APP_NAME " v" APP_VERSION, TFT_WHITE, 2);
#else
    displayPrint(4, 4, APP_NAME, TFT_WHITE, 1);
    displayPrint(4, 16, "v" APP_VERSION, TFT_CYAN, 1);
#endif

    wifiInit();

    if (wifiIsConnected()) {
        displayPrint(10, 40, "WiFi: OK", TFT_GREEN, 1);
        displayPrint(10, 55, wifiGetIP().c_str(), TFT_CYAN, 1);
    } else {
        displayPrint(10, 40, "WiFi: offline", TFT_RED, 1);
    }

    int bootCount = storageGetInt("boot_count", 0);
    bootCount++;
    storageSetInt("boot_count", bootCount);
    char bootStr[32];
    sprintf(bootStr, "Boot count: %d", bootCount);
    displayPrint(10, 70, bootStr, TFT_CYAN, 1);
    Serial.printf("[NVS] %s\n", bootStr);

    Serial.println("Boot complete.");
}

void loop() {
    buttonsLoop();
    wifiLoop();
}