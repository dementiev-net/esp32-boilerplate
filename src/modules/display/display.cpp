#include "display.h"
#include "../board/board_profile.h"
#include <TFT_eSPI.h>

static TFT_eSPI tft = TFT_eSPI();

void displayInit() {
    Serial.println("[Display] Initializing...");

    const BoardProfile& board = boardGetProfile();
    if (board.pins.enable5vPin >= 0) {
        pinMode(board.pins.enable5vPin, OUTPUT);
        digitalWrite(board.pins.enable5vPin, HIGH);
        delay(100);
    }

#ifdef TFT_BL
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
#endif

    tft.init();
    tft.setRotation(board.display.rotation);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    Serial.println("[Display] Ready.");
}

void displayClear(uint32_t color) {
    tft.fillScreen(color);
}

void displayPrint(int x, int y, const char* text, uint32_t color, uint8_t size) {
    tft.setTextColor(color, TFT_BLACK);
    tft.setTextSize(size);
    tft.drawString(text, x, y);
}

void displayDrawRect(int x, int y, int w, int h, uint32_t color) {
    tft.drawRect(x, y, w, h, color);
}

void displayFillRect(int x, int y, int w, int h, uint32_t color) {
    tft.fillRect(x, y, w, h, color);
}

void displayDrawLine(int x1, int y1, int x2, int y2, uint32_t color) {
    tft.drawLine(x1, y1, x2, y2, color);
}

void displayStatusBar(const char* left, const char* right) {
    const int screenWidth = boardGetProfile().display.width;
    displayFillRect(0, 0, screenWidth, 16, 0x1082);
    displayPrint(4, 2, left, TFT_WHITE, 1);
    int rightX = screenWidth - (strlen(right) * 6) - 4;
    displayPrint(rightX, 2, right, TFT_WHITE, 1);
}
