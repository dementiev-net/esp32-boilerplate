#include "boot_preloader.h"

#include "../../config.h"
#include "../board/board_profile.h"
#include "../display/display.h"

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

static BootPreloaderUi ui;

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

static void drawStageLabel(const char* stageLabel) {
    displayFillRect(ui.stageX, ui.stageY, ui.stageW, ui.stageH, TFT_BLACK);
    displayPrint(ui.stageX, ui.stageY, stageLabel, TFT_CYAN, 1);
}

static void drawProgress(uint8_t progressPercent) {
    if (progressPercent > 100) {
        progressPercent = 100;
    }

    const int barInnerW = ui.barW - 2;
    const int oldFillW = (barInnerW * ui.progress) / 100;
    const int newFillW = (barInnerW * progressPercent) / 100;

    if (newFillW > oldFillW) {
        displayFillRect(
            ui.barX + 1 + oldFillW,
            ui.barY + 1,
            newFillW - oldFillW,
            ui.barH - 2,
            TFT_GREEN
        );
    } else if (newFillW < oldFillW) {
        displayFillRect(
            ui.barX + 1 + newFillW,
            ui.barY + 1,
            oldFillW - newFillW,
            ui.barH - 2,
            TFT_BLACK
        );
    }

    ui.progress = progressPercent;

    displayFillRect(ui.percentX, ui.percentY, ui.percentW, ui.percentH, TFT_BLACK);
    char percentLabel[8];
    snprintf(percentLabel, sizeof(percentLabel), "%3u%%", static_cast<unsigned>(progressPercent));
    displayPrint(ui.percentX, ui.percentY, percentLabel, TFT_WHITE, 1);
}

void bootPreloaderBegin() {
    const BoardProfile& board = boardGetProfile();

    ui.initialized = true;
    ui.startedMs = millis();
    ui.progress = 0;

    ui.logoScale = board.display.width >= 200 ? 4 : 2;
    ui.logoSize = 16 * ui.logoScale;
    ui.logoX = (board.display.width - ui.logoSize) / 2;
    ui.logoY = board.display.width >= 200 ? 8 : 6;

    ui.barX = 8;
    ui.barH = board.display.width >= 200 ? 12 : 10;
    ui.barW = board.display.width - 16;
    ui.barY = board.display.height - ui.barH - 10;

    ui.stageX = 8;
    ui.stageY = ui.barY - 12;
    ui.stageW = board.display.width - 54;
    ui.stageH = 10;

    ui.percentX = ui.barX + ui.barW - 34;
    ui.percentY = ui.stageY;

    displayClear(TFT_BLACK);
    drawBootLogo(ui.logoX, ui.logoY, ui.logoScale);

    if (board.display.width >= 200) {
        displayPrint(10, ui.logoY + ui.logoSize + 6, APP_NAME " v" APP_VERSION, TFT_WHITE, 2);
    } else {
        displayPrint(4, ui.logoY + ui.logoSize + 4, APP_NAME, TFT_WHITE, 1);
        displayPrint(4, ui.logoY + ui.logoSize + 16, "v" APP_VERSION, TFT_CYAN, 1);
    }

    displayDrawRect(ui.barX, ui.barY, ui.barW, ui.barH, 0x7BEF);
    drawStageLabel("Booting");
    drawProgress(0);
}

void bootPreloaderStep(uint8_t progressPercent, const char* stageLabel, bool smooth) {
    if (!ui.initialized) {
        bootPreloaderBegin();
    }

    drawStageLabel(stageLabel);

    if (!smooth || progressPercent <= ui.progress) {
        drawProgress(progressPercent);
        return;
    }

    while (ui.progress < progressPercent) {
        drawProgress(static_cast<uint8_t>(ui.progress + 1));
        delay(5);
    }
}

void bootPreloaderFinish() {
    bootPreloaderStep(100, "Starting", true);

    const unsigned long elapsedMs = millis() - ui.startedMs;
    if (elapsedMs >= BOOT_SPLASH_MIN_MS) {
        return;
    }

    const unsigned long finishAtMs = ui.startedMs + BOOT_SPLASH_MIN_MS;
    uint8_t dots = 0;
    while (isFutureOrNow(finishAtMs, millis())) {
        char stageLabel[20];
        snprintf(stageLabel, sizeof(stageLabel), "Starting%.*s", dots, "...");
        drawStageLabel(stageLabel);
        dots = (dots + 1) % 4;
        delay(120);
    }
}
