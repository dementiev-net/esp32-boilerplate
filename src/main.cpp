#include <Arduino.h>

#include "config.h"
#include "modules/app/app_runtime.h"
#include "modules/app/boot_preloader.h"
#include "modules/app/runtime_state.h"
#include "modules/app/status_panel.h"
#include "modules/board/board_profile.h"
#include "modules/buttons/buttons.h"
#include "modules/display/display.h"
#include "modules/net/net_http_demo.h"
#include "modules/power/battery.h"
#include "modules/power/sleep.h"
#include "modules/system/reliability.h"
#include "modules/storage/storage.h"
#include "modules/time/net_time.h"
#include "modules/usb/usb_shell.h"
#include "modules/wifi/wifi.h"
#if FEATURE_BLE
#include "modules/ble/ble.h"
#endif
#if FEATURE_OTA
#include "modules/ota/ota.h"
#endif

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

    reliabilityInit();
    appRuntimeInit();
    runtimeStateInit(runtimeTracker);
    sleepInit();

    displayInit();
    bootPreloaderBegin();

    bootPreloaderStep(20, "Storage init");
    storageInit();

    bootPreloaderStep(28, "Power init");
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

    #if FEATURE_BLE
    bootPreloaderStep(76, "BLE init");
    bleInit();
    #else
    Serial.println("[BLE] Disabled by build profile");
    #endif

    #if FEATURE_OTA
    bootPreloaderStep(82, "OTA init");
    otaInit();
    #else
    Serial.println("[OTA] Disabled by build profile");
    #endif

    #if FEATURE_USB_SHELL
    bootPreloaderStep(88, "USB shell");
    usbShellInit();
    #else
    Serial.println("[USB] Shell disabled by build profile");
    #endif

    bootPreloaderStep(92, "Time init");
    netTimeInit();

    #if FEATURE_NET_HTTP
    bootPreloaderStep(96, "Net demo");
    netDemoInit();
    #else
    Serial.println("[NET] HTTP demo disabled by build profile");
    #endif

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

    char wakeStr[40];
    snprintf(wakeStr, sizeof(wakeStr), "Wake: %s", sleepWakeReasonLabel());
    if (board.display.width >= 200) {
        char resetStr[44];
        snprintf(
            resetStr,
            sizeof(resetStr),
            "Reset: %s",
            reliabilityResetReasonLabel()
        );
        displayPrint(10, 46, resetStr, 0x7BEF, 1);
        displayPrint(10, 58, wakeStr, 0x7BEF, 1);
    } else {
        displayPrint(4, 28, wakeStr, 0x7BEF, 1);
    }

    Serial.printf(
        "[System] Reset: %s (%d)\n",
        reliabilityResetReasonLabel(),
        reliabilityResetReasonCode()
    );
    Serial.printf(
        "[System] WDT: %s timeout=%us\n",
        reliabilityWatchdogEnabled() ? "on" : "off",
        static_cast<unsigned>(WATCHDOG_TIMEOUT_SEC)
    );
    Serial.printf("[Power] %s\n", wakeStr);
    if (sleepCanWakeByButton()) {
        Serial.printf("[Power] Wake button: TOP GPIO%d\n", sleepWakeButtonPin());
    } else {
        Serial.println("[Power] Wake button is not available for this board");
    }

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
    reliabilityLoop();
    buttonsLoop();
    wifiLoop();
    #if FEATURE_USB_SHELL
    usbShellLoop();
    #endif
    #if FEATURE_BLE
    bleLoop();
    #endif
    #if FEATURE_OTA
    otaLoop();
    #endif
    netTimeLoop(wifiIsConnected() && !wifiIsApMode());
    netDemoLoop(wifiIsConnected() && !wifiIsApMode());

    const unsigned long nowMs = millis();
    RuntimeSnapshot sampledState = runtimeStateRead(runtimeTracker, nowMs);
    #if FEATURE_BLE
    bleSetTelemetry(sampledState.batteryPercent, nowMs / 1000UL);
    #endif

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
