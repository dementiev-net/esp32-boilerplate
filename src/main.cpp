#include <Arduino.h>

#include "config.h"
#include "modules/app/app_runtime.h"
#include "modules/app/boot_preloader.h"
#include "modules/app/runtime_state.h"
#include "modules/app/status_panel.h"
#include "modules/board/board_profile.h"
#include "modules/buttons/buttons.h"
#include "modules/display/display.h"
#include "modules/power/battery.h"
#include "modules/storage/storage.h"
#include "modules/time/net_time.h"
#include "modules/wifi/wifi.h"

static bool runtimeInitialized = false;
static bool dispatcherOverflowReported = false;
static RuntimeStateTracker runtimeTracker;
static RuntimeSnapshot runtimeState;

static void postAppEvent(AppEventType type) {
    if (appRuntimePostEvent(type)) {
        return;
    }

    if (!dispatcherOverflowReported) {
        dispatcherOverflowReported = true;
        Serial.printf(
            "[Dispatcher] Event queue overflow. dropped=%u\n",
            static_cast<unsigned>(appRuntimeDroppedEvents())
        );
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println("Booting...");

    appRuntimeInit();
    runtimeStateInit(runtimeTracker);

    displayInit();
    bootPreloaderBegin();

    bootPreloaderStep(20, "Storage init");
    storageInit();

    bootPreloaderStep(30, "Power init");
    batteryInit();

    bootPreloaderStep(40, "Buttons init");
    buttonsInit();

    buttonsOnTopClick([]() {
        postAppEvent(AppEventType::ButtonTopClick);
    });
    buttonsOnTopLongPress([]() {
        postAppEvent(AppEventType::ButtonTopLongPress);
    });
    buttonsOnTopHold([]() {
        postAppEvent(AppEventType::ButtonTopHold);
    });

    buttonsOnBottomClick([]() {
        postAppEvent(AppEventType::ButtonBottomClick);
    });
    buttonsOnBottomLongPress([]() {
        postAppEvent(AppEventType::ButtonBottomLongPress);
    });
    buttonsOnBottomHold([]() {
        postAppEvent(AppEventType::ButtonBottomHold);
    });

    bootPreloaderStep(55, "Events bind");

    bootPreloaderStep(70, "Wi-Fi init");
    wifiInit();

    bootPreloaderStep(85, "Time init");
    netTimeInit();

    int bootCount = storageGetInt("boot_count", 0);
    bootCount++;
    storageSetInt("boot_count", bootCount);
    bootPreloaderFinish();

    displayClear(TFT_BLACK);
    const BoardProfile& board = boardGetProfile();
    if (board.display.width >= 200) {
        displayPrint(10, 10, APP_NAME " v" APP_VERSION, TFT_WHITE, 2);
    } else {
        displayPrint(4, 4, APP_NAME, TFT_WHITE, 1);
        displayPrint(4, 16, "v" APP_VERSION, TFT_CYAN, 1);
    }

    char bootStr[32];
    snprintf(bootStr, sizeof(bootStr), "Boot count: %d", bootCount);
    displayPrint(10, 70, bootStr, TFT_CYAN, 1);
    Serial.printf("[NVS] %s\n", bootStr);

    statusPanelReset();

    runtimeState = runtimeStateRead(runtimeTracker, millis());
    runtimeInitialized = true;
    postAppEvent(AppEventType::UiRefreshRequested);

    AppRuntimeDrainResult drainResult = appRuntimeDrainEvents(runtimeTracker, runtimeState, millis());
    if (drainResult.needUiRefresh) {
        runtimeState = runtimeStateRead(runtimeTracker, millis());
        statusPanelRender(runtimeState, true);
    }

    Serial.println("Boot complete.");
}

void loop() {
    buttonsLoop();
    wifiLoop();
    netTimeLoop(wifiIsConnected() && !wifiIsApMode());

    const unsigned long nowMs = millis();
    RuntimeSnapshot sampledState = runtimeStateRead(runtimeTracker, nowMs);

    if (!runtimeInitialized) {
        runtimeState = sampledState;
        runtimeInitialized = true;
        postAppEvent(AppEventType::UiRefreshRequested);
    } else if (!runtimeStateEquals(runtimeState, sampledState)) {
        appRuntimePublishStateChanges(runtimeState, sampledState);
        runtimeState = sampledState;
    }

    AppRuntimeDrainResult drainResult = appRuntimeDrainEvents(runtimeTracker, runtimeState, nowMs);
    if (drainResult.needUiRefresh && runtimeInitialized) {
        runtimeState = runtimeStateRead(runtimeTracker, millis());
        statusPanelRender(runtimeState);
    }
}
