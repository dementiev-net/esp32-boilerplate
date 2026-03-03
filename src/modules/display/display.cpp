#include "display.h"
#include "../../config.h"
#include <TFT_eSPI.h>

static TFT_eSPI tft = TFT_eSPI();

void displayInit() {
    Serial.println("[Display] Initializing...");

#ifdef BOARD_TQT_PRO
    pinMode(4, OUTPUT);
    digitalWrite(4, HIGH);
    delay(100);
#endif

#ifdef TFT_BL
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
#endif

    tft.init();
    tft.setRotation(SCREEN_ROTATION);
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
    displayFillRect(0, 0, SCREEN_WIDTH, 16, 0x1082);
    displayPrint(4, 2, left, TFT_WHITE, 1);
    int rightX = SCREEN_WIDTH - (strlen(right) * 6) - 4;
    displayPrint(rightX, 2, right, TFT_WHITE, 1);
}