#include <Arduino.h>
#include "config.h"
#include "modules/display/display.h"
#include "modules/wifi/wifi.h"
#include "modules/storage/storage.h"
#include "modules/buttons/buttons.h"

static unsigned long lastStatusRefreshMs = 0;
static bool statusInitialized = false;
static bool lastTopPressed = false;
static bool lastBottomPressed = false;
static bool lastWifiConnected = false;
static bool lastSdSupported = false;
static bool lastSdAvailable = false;
static String lastIp = "";

// Обновляет нижний статусный блок только при изменении состояния,
// чтобы не перерисовывать экран лишний раз в каждом цикле loop().
static void displayRuntimeStatus(bool force = false) {
    const bool topPressed = buttonsIsTopPressed();
    const bool bottomPressed = buttonsIsBottomPressed();
    const bool wifiConnected = wifiIsConnected();
    const bool sdIsSupported = sdSupported();
    const bool sdReady = sdAvailable();
    const String ip = wifiConnected ? wifiGetIP() : "-";

    if (!force && statusInitialized
        && topPressed == lastTopPressed
        && bottomPressed == lastBottomPressed
        && wifiConnected == lastWifiConnected
        && sdIsSupported == lastSdSupported
        && sdReady == lastSdAvailable
        && ip == lastIp) {
        return;
    }

    statusInitialized = true;
    lastTopPressed = topPressed;
    lastBottomPressed = bottomPressed;
    lastWifiConnected = wifiConnected;
    lastSdSupported = sdIsSupported;
    lastSdAvailable = sdReady;
    lastIp = ip;

    const int statusHeight = 36;
    const int statusY = SCREEN_HEIGHT - statusHeight;
    const uint16_t gray = 0x7BEF;

    displayFillRect(0, statusY, SCREEN_WIDTH, statusHeight, TFT_BLACK);

    char line1[32];
    snprintf(
        line1,
        sizeof(line1),
        "W:%s M:%s SD:%s",
        wifiConnected ? "OK" : "OFF",
        wifiGetModeLabel(),
        sdIsSupported ? (sdReady ? "OK" : "NO") : "N/A"
    );
    displayPrint(2, statusY, line1, wifiConnected ? TFT_GREEN : TFT_RED, 1);

    char line2[32];
    snprintf(line2, sizeof(line2), "BTN T:%d B:%d", topPressed ? 1 : 0, bottomPressed ? 1 : 0);
    displayPrint(2, statusY + 12, line2, (topPressed || bottomPressed) ? TFT_YELLOW : TFT_WHITE, 1);

    char line3[32];
    if (wifiIsStaMode()) {
        snprintf(line3, sizeof(line3), "IP:%s", wifiConnected ? ip.c_str() : "-");
        displayPrint(2, statusY + 24, line3, wifiConnected ? TFT_CYAN : gray, 1);
    } else if (wifiIsApMode()) {
        snprintf(line3, sizeof(line3), "AP:%s", wifiGetSSID().c_str());
        displayPrint(2, statusY + 24, line3, TFT_CYAN, 1);
    } else {
        snprintf(line3, sizeof(line3), "IP:%s", wifiConnected ? ip.c_str() : "-");
        displayPrint(2, statusY + 24, line3, wifiConnected ? TFT_CYAN : gray, 1);
    }
}

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
        Serial.println("[Button] Top long press");
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

    int bootCount = storageGetInt("boot_count", 0);
    bootCount++;
    storageSetInt("boot_count", bootCount);
    char bootStr[32];
    sprintf(bootStr, "Boot count: %d", bootCount);
    displayPrint(10, 70, bootStr, TFT_CYAN, 1);
    Serial.printf("[NVS] %s\n", bootStr);

    displayRuntimeStatus(true);
    Serial.println("Boot complete.");
}

void loop() {
    buttonsLoop();
    wifiLoop();

    const unsigned long now = millis();
    if (now - lastStatusRefreshMs >= 100) {
        lastStatusRefreshMs = now;
        displayRuntimeStatus();
    }
}
