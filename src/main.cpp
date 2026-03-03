#include <Arduino.h>

#include "config.h"
#include "modules/app/app_dispatcher.h"
#include "modules/board/board_profile.h"
#include "modules/buttons/buttons.h"
#include "modules/display/display.h"
#include "modules/storage/storage.h"
#include "modules/time/net_time.h"
#include "modules/wifi/wifi.h"

static bool statusInitialized = false;
static bool dispatcherOverflowReported = false;
static unsigned long topHoldUntilMs = 0;
static unsigned long bottomHoldUntilMs = 0;
static String cachedUiTime = "--:--:--";
static unsigned long lastTimeUiSampleMs = 0;

static const char* kBootLogo16[16] = {
    "................",
    "......22........",
    ".....2222.......",
    "...22222222.....",
    "..222....222....",
    ".22........22...",
    ".2..444444..2...",
    ".2..4....4..2...",
    ".2..4.33.4..2...",
    ".2..4....4..2...",
    ".2..444444..2...",
    ".22........22...",
    "..222....222....",
    "...22222222.....",
    ".....2222.......",
    "......22........"
};

struct BootPreloaderUi {
    bool initialized = false;
    unsigned long startedMs = 0;
    uint8_t progress = 0;
    int logoX = 0;
    int logoY = 0;
    int logoScale = 0;
    int logoSize = 0;
    int stageX = 0;
    int stageY = 0;
    int stageW = 0;
    int stageH = 0;
    int barX = 0;
    int barY = 0;
    int barW = 0;
    int barH = 0;
    int percentX = 0;
    int percentY = 0;
    int percentW = 40;
    int percentH = 10;
};

static BootPreloaderUi bootUi;

struct RuntimeSnapshot {
    bool topPressed = false;
    bool bottomPressed = false;
    bool topHoldActive = false;
    bool bottomHoldActive = false;
    bool wifiConnected = false;
    bool sdSupported = false;
    bool sdAvailable = false;
    String modeLabel = "";
    String ip = "-";
    String ssid = "";
    bool timeSynced = false;
    String localTime = "--:--:--";
};

struct StatusLineCache {
    bool initialized = false;
    String text = "";
    uint16_t color = TFT_WHITE;
};

static RuntimeSnapshot lastRenderedState;
static RuntimeSnapshot runtimeState;
static bool runtimeInitialized = false;
static StatusLineCache statusLineCache[4];

static bool isFutureOrNow(unsigned long nowMs, unsigned long deadlineMs) {
    return static_cast<long>(deadlineMs - nowMs) >= 0;
}

static uint16_t bootLogoColor(char code) {
    switch (code) {
        case '2':
            return TFT_CYAN;
        case '3':
            return TFT_YELLOW;
        case '4':
            return TFT_WHITE;
        default:
            return TFT_BLACK;
    }
}

static void drawBootLogo(int x, int y, int scale) {
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            const char code = kBootLogo16[row][col];
            if (code == '.') {
                continue;
            }
            displayFillRect(
                x + col * scale,
                y + row * scale,
                scale,
                scale,
                bootLogoColor(code)
            );
        }
    }
}

static void bootPreloaderDrawStage(const char* stageLabel) {
    displayFillRect(bootUi.stageX, bootUi.stageY, bootUi.stageW, bootUi.stageH, TFT_BLACK);
    displayPrint(bootUi.stageX, bootUi.stageY, stageLabel, TFT_CYAN, 1);
}

static void bootPreloaderDrawProgress(uint8_t progressPercent) {
    if (progressPercent > 100) {
        progressPercent = 100;
    }

    const int barInnerW = bootUi.barW - 2;
    const int oldFillW = (barInnerW * bootUi.progress) / 100;
    const int newFillW = (barInnerW * progressPercent) / 100;

    if (newFillW > oldFillW) {
        displayFillRect(
            bootUi.barX + 1 + oldFillW,
            bootUi.barY + 1,
            newFillW - oldFillW,
            bootUi.barH - 2,
            TFT_GREEN
        );
    } else if (newFillW < oldFillW) {
        displayFillRect(
            bootUi.barX + 1 + newFillW,
            bootUi.barY + 1,
            oldFillW - newFillW,
            bootUi.barH - 2,
            TFT_BLACK
        );
    }

    bootUi.progress = progressPercent;

    displayFillRect(bootUi.percentX, bootUi.percentY, bootUi.percentW, bootUi.percentH, TFT_BLACK);
    char percentLabel[8];
    snprintf(percentLabel, sizeof(percentLabel), "%3u%%", static_cast<unsigned>(progressPercent));
    displayPrint(bootUi.percentX, bootUi.percentY, percentLabel, TFT_WHITE, 1);
}

static void bootPreloaderBegin() {
    const BoardProfile& board = boardGetProfile();

    bootUi.initialized = true;
    bootUi.startedMs = millis();
    bootUi.progress = 0;

    bootUi.logoScale = board.display.width >= 200 ? 4 : 2;
    bootUi.logoSize = 16 * bootUi.logoScale;
    bootUi.logoX = (board.display.width - bootUi.logoSize) / 2;
    bootUi.logoY = board.display.width >= 200 ? 8 : 6;

    bootUi.barX = 8;
    bootUi.barH = board.display.width >= 200 ? 12 : 10;
    bootUi.barW = board.display.width - 16;
    bootUi.barY = board.display.height - bootUi.barH - 10;

    bootUi.stageX = 8;
    bootUi.stageY = bootUi.barY - 12;
    bootUi.stageW = board.display.width - 54;
    bootUi.stageH = 10;

    bootUi.percentX = bootUi.barX + bootUi.barW - 34;
    bootUi.percentY = bootUi.stageY;

    displayClear(TFT_BLACK);
    drawBootLogo(bootUi.logoX, bootUi.logoY, bootUi.logoScale);

    if (board.display.width >= 200) {
        displayPrint(10, bootUi.logoY + bootUi.logoSize + 6, APP_NAME " v" APP_VERSION, TFT_WHITE, 2);
    } else {
        displayPrint(4, bootUi.logoY + bootUi.logoSize + 4, APP_NAME, TFT_WHITE, 1);
        displayPrint(4, bootUi.logoY + bootUi.logoSize + 16, "v" APP_VERSION, TFT_CYAN, 1);
    }

    displayDrawRect(bootUi.barX, bootUi.barY, bootUi.barW, bootUi.barH, 0x7BEF);
    bootPreloaderDrawStage("Booting");
    bootPreloaderDrawProgress(0);
}

static void bootPreloaderStep(uint8_t progressPercent, const char* stageLabel, bool smooth) {
    if (!bootUi.initialized) {
        bootPreloaderBegin();
    }

    bootPreloaderDrawStage(stageLabel);

    if (!smooth || progressPercent <= bootUi.progress) {
        bootPreloaderDrawProgress(progressPercent);
        return;
    }

    while (bootUi.progress < progressPercent) {
        bootPreloaderDrawProgress(static_cast<uint8_t>(bootUi.progress + 1));
        delay(5);
    }
}

static void bootPreloaderFinish() {
    bootPreloaderStep(100, "Starting", true);

    const unsigned long elapsedMs = millis() - bootUi.startedMs;
    if (elapsedMs >= BOOT_SPLASH_MIN_MS) {
        return;
    }

    const unsigned long finishAtMs = bootUi.startedMs + BOOT_SPLASH_MIN_MS;
    uint8_t dots = 0;
    while (isFutureOrNow(finishAtMs, millis())) {
        char stageLabel[20];
        snprintf(stageLabel, sizeof(stageLabel), "Starting%.*s", dots, "...");
        bootPreloaderDrawStage(stageLabel);
        dots = (dots + 1) % 4;
        delay(120);
    }
}

static RuntimeSnapshot readRuntimeSnapshot() {
    const unsigned long nowMs = millis();
    RuntimeSnapshot state;

    state.topPressed = buttonsIsTopPressed();
    state.bottomPressed = buttonsIsBottomPressed();
    state.topHoldActive = isFutureOrNow(nowMs, topHoldUntilMs);
    state.bottomHoldActive = isFutureOrNow(nowMs, bottomHoldUntilMs);
    state.wifiConnected = wifiIsConnected();
    state.sdSupported = sdSupported();
    state.sdAvailable = sdAvailable();
    state.modeLabel = wifiGetModeLabel();
    state.ip = state.wifiConnected ? wifiGetIP() : "-";
    state.ssid = wifiGetSSID();
    state.timeSynced = netTimeIsSynced();

    if (!state.timeSynced) {
        cachedUiTime = "--:--:--";
    } else if (nowMs - lastTimeUiSampleMs >= 1000 || cachedUiTime == "--:--:--") {
        cachedUiTime = netTimeGetLocalTimeText();
        lastTimeUiSampleMs = nowMs;
    }
    state.localTime = cachedUiTime;

    return state;
}

static bool snapshotsEqual(const RuntimeSnapshot& a, const RuntimeSnapshot& b) {
    return a.topPressed == b.topPressed
        && a.bottomPressed == b.bottomPressed
        && a.topHoldActive == b.topHoldActive
        && a.bottomHoldActive == b.bottomHoldActive
        && a.wifiConnected == b.wifiConnected
        && a.sdSupported == b.sdSupported
        && a.sdAvailable == b.sdAvailable
        && a.modeLabel == b.modeLabel
        && a.ip == b.ip
        && a.ssid == b.ssid
        && a.timeSynced == b.timeSynced
        && a.localTime == b.localTime;
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

static void statusLineRender(
    const BoardProfile& board,
    uint8_t lineIndex,
    int y,
    const String& text,
    uint16_t color,
    bool force
) {
    StatusLineCache& cache = statusLineCache[lineIndex];
    if (!force && cache.initialized && cache.text == text && cache.color == color) {
        return;
    }

    displayFillRect(0, y, board.display.width, 12, TFT_BLACK);
    displayPrint(2, y, text.c_str(), color, 1);

    cache.initialized = true;
    cache.text = text;
    cache.color = color;
}

// Обновляет нижний статусный блок построчно, без полной перерисовки области.
static void displayRuntimeStatus(const RuntimeSnapshot& state, bool force = false) {
    if (!force && statusInitialized && snapshotsEqual(lastRenderedState, state)) {
        return;
    }

    const BoardProfile& board = boardGetProfile();
    const int statusHeight = 48;
    const int statusY = board.display.height - statusHeight;
    const uint16_t gray = 0x7BEF;

    if (!statusInitialized) {
        displayFillRect(0, statusY, board.display.width, statusHeight, TFT_BLACK);
        for (uint8_t i = 0; i < 4; i++) {
            statusLineCache[i].initialized = false;
            statusLineCache[i].text = "";
            statusLineCache[i].color = TFT_WHITE;
        }
    }

    char line1Buffer[32];
    snprintf(
        line1Buffer,
        sizeof(line1Buffer),
        "W:%s M:%s SD:%s",
        state.wifiConnected ? "OK" : "OFF",
        state.modeLabel.c_str(),
        state.sdSupported ? (state.sdAvailable ? "OK" : "NO") : "N/A"
    );
    statusLineRender(
        board,
        0,
        statusY,
        String(line1Buffer),
        state.wifiConnected ? TFT_GREEN : TFT_RED,
        force
    );

    char line2Buffer[32];
    snprintf(
        line2Buffer,
        sizeof(line2Buffer),
        "BTN T:%d%s B:%d%s",
        state.topPressed ? 1 : 0,
        state.topHoldActive ? "H" : "",
        state.bottomPressed ? 1 : 0,
        state.bottomHoldActive ? "H" : ""
    );
    statusLineRender(
        board,
        1,
        statusY + 12,
        String(line2Buffer),
        (state.topPressed || state.bottomPressed) ? TFT_YELLOW : TFT_WHITE,
        force
    );

    char line3Buffer[32];
    if (state.modeLabel == "STA") {
        snprintf(line3Buffer, sizeof(line3Buffer), "IP:%s", state.wifiConnected ? state.ip.c_str() : "-");
        statusLineRender(
            board,
            2,
            statusY + 24,
            String(line3Buffer),
            state.wifiConnected ? TFT_CYAN : gray,
            force
        );
    } else if (state.modeLabel == "AP") {
        snprintf(line3Buffer, sizeof(line3Buffer), "AP:%s", state.ssid.c_str());
        statusLineRender(board, 2, statusY + 24, String(line3Buffer), TFT_CYAN, force);
    } else {
        snprintf(line3Buffer, sizeof(line3Buffer), "IP:%s", state.wifiConnected ? state.ip.c_str() : "-");
        statusLineRender(
            board,
            2,
            statusY + 24,
            String(line3Buffer),
            state.wifiConnected ? TFT_CYAN : gray,
            force
        );
    }

    char line4Buffer[32];
    snprintf(line4Buffer, sizeof(line4Buffer), "TIME:%s", state.localTime.c_str());
    statusLineRender(
        board,
        3,
        statusY + 36,
        String(line4Buffer),
        state.timeSynced ? TFT_YELLOW : gray,
        force
    );

    statusInitialized = true;
    lastRenderedState = state;
}

static void publishStateChangeEvents(const RuntimeSnapshot& previous, const RuntimeSnapshot& current) {
    bool needUiRefresh = false;

    if (previous.topPressed != current.topPressed || previous.bottomPressed != current.bottomPressed) {
        postAppEvent(AppEventType::ButtonsStateChanged);
        needUiRefresh = true;
    }

    if (previous.topHoldActive != current.topHoldActive || previous.bottomHoldActive != current.bottomHoldActive) {
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

    if (previous.timeSynced != current.timeSynced || previous.localTime != current.localTime) {
        postAppEvent(AppEventType::TimeUpdated);
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
            case AppEventType::ButtonTopHold:
                topHoldUntilMs = millis() + BUTTON_HOLD_INDICATOR_MS;
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
            case AppEventType::ButtonBottomHold:
                bottomHoldUntilMs = millis() + BUTTON_HOLD_INDICATOR_MS;
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
            case AppEventType::TimeUpdated:
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

    displayInit();
    bootPreloaderBegin();

    bootPreloaderStep(20, "Storage init", true);
    storageInit();

    bootPreloaderStep(35, "Buttons init", true);
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
    bootPreloaderStep(50, "Events bind", true);

    bootPreloaderStep(65, "Wi-Fi init", true);
    wifiInit();

    bootPreloaderStep(80, "Time init", true);
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

    runtimeState = readRuntimeSnapshot();
    runtimeInitialized = true;
    postAppEvent(AppEventType::UiRefreshRequested);
    processAppEvents();
    Serial.println("Boot complete.");
}

void loop() {
    buttonsLoop();
    wifiLoop();
    netTimeLoop(wifiIsConnected() && !wifiIsApMode());

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
