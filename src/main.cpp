#include <Arduino.h>
#include "config.h"
#include "modules/app/app_dispatcher.h"
#include "modules/board/board_profile.h"
#include "modules/display/display.h"
#include "modules/wifi/wifi.h"
#include "modules/storage/storage.h"
#include "modules/buttons/buttons.h"

static bool statusInitialized = false;
static bool dispatcherOverflowReported = false;

struct RuntimeSnapshot {
    bool topPressed = false;
    bool bottomPressed = false;
    bool wifiConnected = false;
    bool sdSupported = false;
    bool sdAvailable = false;
    String modeLabel = "";
    String ip = "-";
    String ssid = "";
};

static RuntimeSnapshot lastRenderedState;
static RuntimeSnapshot runtimeState;
static bool runtimeInitialized = false;

static RuntimeSnapshot readRuntimeSnapshot() {
    RuntimeSnapshot state;
    state.topPressed = buttonsIsTopPressed();
    state.bottomPressed = buttonsIsBottomPressed();
    state.wifiConnected = wifiIsConnected();
    state.sdSupported = sdSupported();
    state.sdAvailable = sdAvailable();
    state.modeLabel = wifiGetModeLabel();
    state.ip = state.wifiConnected ? wifiGetIP() : "-";
    state.ssid = wifiGetSSID();
    return state;
}

static bool snapshotsEqual(const RuntimeSnapshot& a, const RuntimeSnapshot& b) {
    return a.topPressed == b.topPressed
        && a.bottomPressed == b.bottomPressed
        && a.wifiConnected == b.wifiConnected
        && a.sdSupported == b.sdSupported
        && a.sdAvailable == b.sdAvailable
        && a.modeLabel == b.modeLabel
        && a.ip == b.ip
        && a.ssid == b.ssid;
}

static void postAppEvent(AppEventType type) {
    if (appDispatcherPost(type)) {
        return;
    }

    if (!dispatcherOverflowReported) {
        dispatcherOverflowReported = true;
        Serial.println("[Dispatcher] Event queue overflow. Some events were dropped.");
    }
}

// Обновляет нижний статусный блок только при изменении состояния,
// чтобы не перерисовывать экран лишний раз в каждом цикле loop().
static void displayRuntimeStatus(const RuntimeSnapshot& state, bool force = false) {
    if (!force && statusInitialized && snapshotsEqual(lastRenderedState, state)) {
        return;
    }

    statusInitialized = true;
    lastRenderedState = state;

    const int statusHeight = 36;
    const BoardProfile& board = boardGetProfile();
    const int statusY = board.display.height - statusHeight;
    const uint16_t gray = 0x7BEF;

    displayFillRect(0, statusY, board.display.width, statusHeight, TFT_BLACK);

    char line1[32];
    snprintf(
        line1,
        sizeof(line1),
        "W:%s M:%s SD:%s",
        state.wifiConnected ? "OK" : "OFF",
        state.modeLabel.c_str(),
        state.sdSupported ? (state.sdAvailable ? "OK" : "NO") : "N/A"
    );
    displayPrint(2, statusY, line1, state.wifiConnected ? TFT_GREEN : TFT_RED, 1);

    char line2[32];
    snprintf(line2, sizeof(line2), "BTN T:%d B:%d", state.topPressed ? 1 : 0, state.bottomPressed ? 1 : 0);
    displayPrint(2, statusY + 12, line2, (state.topPressed || state.bottomPressed) ? TFT_YELLOW : TFT_WHITE, 1);

    char line3[32];
    if (state.modeLabel == "STA") {
        snprintf(line3, sizeof(line3), "IP:%s", state.wifiConnected ? state.ip.c_str() : "-");
        displayPrint(2, statusY + 24, line3, state.wifiConnected ? TFT_CYAN : gray, 1);
    } else if (state.modeLabel == "AP") {
        snprintf(line3, sizeof(line3), "AP:%s", state.ssid.c_str());
        displayPrint(2, statusY + 24, line3, TFT_CYAN, 1);
    } else {
        snprintf(line3, sizeof(line3), "IP:%s", state.wifiConnected ? state.ip.c_str() : "-");
        displayPrint(2, statusY + 24, line3, state.wifiConnected ? TFT_CYAN : gray, 1);
    }
}

static void publishStateChangeEvents(const RuntimeSnapshot& previous, const RuntimeSnapshot& current) {
    bool needUiRefresh = false;

    if (previous.topPressed != current.topPressed || previous.bottomPressed != current.bottomPressed) {
        postAppEvent(AppEventType::ButtonsStateChanged);
        needUiRefresh = true;
    }

    if (previous.wifiConnected != current.wifiConnected) {
        postAppEvent(AppEventType::WifiConnectionChanged);
        needUiRefresh = true;
    }

    if (previous.modeLabel != current.modeLabel || previous.ip != current.ip || previous.ssid != current.ssid) {
        postAppEvent(AppEventType::WifiIdentityChanged);
        needUiRefresh = true;
    }

    if (previous.sdSupported != current.sdSupported || previous.sdAvailable != current.sdAvailable) {
        postAppEvent(AppEventType::SdStateChanged);
        needUiRefresh = true;
    }

    if (needUiRefresh) {
        postAppEvent(AppEventType::UiRefreshRequested);
    }
}

static void processAppEvents() {
    bool needUiRefresh = false;
    AppEvent event;

    while (appDispatcherNext(event)) {
        switch (event.type) {
            case AppEventType::ButtonTopClick:
                Serial.println("[Button] Top click");
                needUiRefresh = true;
                break;
            case AppEventType::ButtonTopLongPress:
                Serial.println("[Button] Top long press");
                needUiRefresh = true;
                break;
            case AppEventType::ButtonBottomClick:
                Serial.println("[Button] Bottom click");
                needUiRefresh = true;
                break;
            case AppEventType::ButtonBottomLongPress:
                Serial.println("[Button] Bottom long press");
                needUiRefresh = true;
                break;
            case AppEventType::ButtonsStateChanged:
                Serial.printf(
                    "[Runtime] Buttons state T:%d B:%d\n",
                    runtimeState.topPressed ? 1 : 0,
                    runtimeState.bottomPressed ? 1 : 0
                );
                break;
            case AppEventType::WifiConnectionChanged:
                Serial.printf(
                    "[Runtime] WiFi %s\n",
                    runtimeState.wifiConnected ? "connected" : "disconnected"
                );
                break;
            case AppEventType::WifiIdentityChanged:
                Serial.printf(
                    "[Runtime] WiFi mode=%s ssid=%s ip=%s\n",
                    runtimeState.modeLabel.c_str(),
                    runtimeState.ssid.c_str(),
                    runtimeState.ip.c_str()
                );
                break;
            case AppEventType::SdStateChanged:
                Serial.printf(
                    "[Runtime] SD supported=%d ready=%d\n",
                    runtimeState.sdSupported ? 1 : 0,
                    runtimeState.sdAvailable ? 1 : 0
                );
                break;
            case AppEventType::UiRefreshRequested:
                needUiRefresh = true;
                break;
            case AppEventType::None:
            default:
                break;
        }
    }

    if (needUiRefresh && runtimeInitialized) {
        displayRuntimeStatus(runtimeState);
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println("Booting...");
    appDispatcherInit();

    storageInit();
    displayInit();
    buttonsInit();

    buttonsOnTopClick([]() {
        postAppEvent(AppEventType::ButtonTopClick);
    });

    buttonsOnTopLongPress([]() {
        postAppEvent(AppEventType::ButtonTopLongPress);
    });

    buttonsOnBottomClick([]() {
        postAppEvent(AppEventType::ButtonBottomClick);
    });

    buttonsOnBottomLongPress([]() {
        postAppEvent(AppEventType::ButtonBottomLongPress);
    });

    displayClear(TFT_BLACK);
    const BoardProfile& board = boardGetProfile();
    if (board.display.width >= 200) {
        displayPrint(10, 10, APP_NAME " v" APP_VERSION, TFT_WHITE, 2);
    } else {
        displayPrint(4, 4, APP_NAME, TFT_WHITE, 1);
        displayPrint(4, 16, "v" APP_VERSION, TFT_CYAN, 1);
    }

    wifiInit();

    int bootCount = storageGetInt("boot_count", 0);
    bootCount++;
    storageSetInt("boot_count", bootCount);
    char bootStr[32];
    sprintf(bootStr, "Boot count: %d", bootCount);
    displayPrint(10, 70, bootStr, TFT_CYAN, 1);
    Serial.printf("[NVS] %s\n", bootStr);

    runtimeState = readRuntimeSnapshot();
    runtimeInitialized = true;
    postAppEvent(AppEventType::UiRefreshRequested);
    processAppEvents();
    Serial.println("Boot complete.");
}

void loop() {
    buttonsLoop();
    wifiLoop();

    const RuntimeSnapshot newState = readRuntimeSnapshot();
    if (!runtimeInitialized) {
        runtimeState = newState;
        runtimeInitialized = true;
        postAppEvent(AppEventType::UiRefreshRequested);
    } else if (!snapshotsEqual(runtimeState, newState)) {
        publishStateChangeEvents(runtimeState, newState);
        runtimeState = newState;
    }

    processAppEvents();
}
