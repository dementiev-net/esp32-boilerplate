#include "status_panel.h"

#include "../board/board_profile.h"
#include "../display/display.h"

struct StatusLineCache {
    bool initialized = false;
    String text = "";
    uint16_t color = TFT_WHITE;
};

static bool panelInitialized = false;
static RuntimeSnapshot lastRenderedState;
static StatusLineCache lineCache[5];

static void statusLineRender(
    const BoardProfile& board,
    uint8_t lineIndex,
    int y,
    const String& text,
    uint16_t color,
    bool force
) {
    StatusLineCache& cache = lineCache[lineIndex];
    if (!force && cache.initialized && cache.text == text && cache.color == color) {
        return;
    }

    displayFillRect(0, y, board.display.width, 12, TFT_BLACK);
    displayPrint(2, y, text.c_str(), color, 1);

    cache.initialized = true;
    cache.text = text;
    cache.color = color;
}

void statusPanelReset() {
    panelInitialized = false;
    for (uint8_t i = 0; i < 5; i++) {
        lineCache[i].initialized = false;
        lineCache[i].text = "";
        lineCache[i].color = TFT_WHITE;
    }
}

void statusPanelRender(const RuntimeSnapshot& state, bool force) {
    if (!force && panelInitialized && runtimeStateEquals(lastRenderedState, state)) {
        return;
    }

    const BoardProfile& board = boardGetProfile();
    const int statusHeight = 48;
    const int statusY = board.display.height - statusHeight;
    const uint16_t gray = 0x7BEF;

    if (!panelInitialized) {
        displayFillRect(0, statusY, board.display.width, statusHeight, TFT_BLACK);
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

    char line4Buffer[40];
    const bool batteryValid = state.batterySupported && state.batteryPercent >= 0;
    if (state.batterySupported) {
        if (batteryValid) {
            if (board.display.width >= 200) {
                snprintf(line4Buffer, sizeof(line4Buffer), "TIME:%s VB:%d%%", state.localTime.c_str(), state.batteryPercent);
            } else {
                snprintf(line4Buffer, sizeof(line4Buffer), "T:%s B:%d%%", state.localTime.c_str(), state.batteryPercent);
            }
        } else if (board.display.width >= 200) {
            snprintf(line4Buffer, sizeof(line4Buffer), "TIME:%s VB:--%%", state.localTime.c_str());
        } else {
            snprintf(line4Buffer, sizeof(line4Buffer), "T:%s B:--%%", state.localTime.c_str());
        }
    } else {
        snprintf(line4Buffer, sizeof(line4Buffer), "TIME:%s", state.localTime.c_str());
    }

    uint16_t line4Color = state.timeSynced ? TFT_YELLOW : gray;
    if (!state.timeSynced && batteryValid) {
        line4Color = TFT_CYAN;
    }
    statusLineRender(
        board,
        3,
        statusY + 36,
        String(line4Buffer),
        line4Color,
        force
    );

    String netLine = state.netUiText;
    uint16_t netColor = 0x7BEF;
    if (!state.netEnabled) {
        netLine = "NET:OFF";
    } else if (!state.wifiConnected || state.modeLabel == "AP") {
        netLine = "NET:OFF";
    } else if (state.netLastRequestOk) {
        netColor = TFT_GREEN;
    } else if (state.netUiText == "NET:-") {
        netColor = 0x7BEF;
    } else {
        netColor = 0xFD20;
    }

    statusLineRender(
        board,
        4,
        statusY - 12,
        netLine,
        netColor,
        force
    );

    panelInitialized = true;
    lastRenderedState = state;
}
