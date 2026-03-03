#include "display.h"

#include "../../config.h"
#include "../board/board_profile.h"
#include <TFT_eSPI.h>

static TFT_eSPI tft = TFT_eSPI();
static bool backlightPwmReady = false;
static uint8_t backlightPercent = DISPLAY_BRIGHTNESS_DEFAULT_PERCENT;

namespace {

#ifdef TFT_BL
#ifndef TFT_BACKLIGHT_ON
#define TFT_BACKLIGHT_ON HIGH
#endif
constexpr uint32_t kPwmMaxDuty = (1UL << DISPLAY_BRIGHTNESS_PWM_BITS) - 1UL;
constexpr bool kBacklightActiveHigh = (TFT_BACKLIGHT_ON == HIGH);

uint32_t brightnessPercentToDuty(uint8_t percent) {
    const uint32_t activeDuty = (static_cast<uint32_t>(percent) * kPwmMaxDuty + 50UL) / 100UL;
    if (kBacklightActiveHigh) {
        return activeDuty;
    }
    return kPwmMaxDuty - activeDuty;
}
#endif

}  // namespace

void displayInit() {
    Serial.println("[Display] Initializing...");

    const BoardProfile& board = boardGetProfile();
    if (board.pins.displayPowerPin >= 0) {
        pinMode(board.pins.displayPowerPin, OUTPUT);
        digitalWrite(board.pins.displayPowerPin, HIGH);
        delay(100);
    }

    tft.init();
    tft.setRotation(board.display.rotation);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TL_DATUM);

#ifdef TFT_BL
    // Важно: настраиваем PWM после tft.init(), т.к. драйвер может
    // менять режим TFT_BL во время собственной инициализации.
    pinMode(TFT_BL, OUTPUT);
    const double configuredFreq = ledcSetup(
        DISPLAY_BRIGHTNESS_PWM_CHANNEL,
        DISPLAY_BRIGHTNESS_PWM_FREQ_HZ,
        DISPLAY_BRIGHTNESS_PWM_BITS
    );
    if (configuredFreq > 0.0) {
        ledcAttachPin(TFT_BL, DISPLAY_BRIGHTNESS_PWM_CHANNEL);
        backlightPwmReady = true;
        displaySetBrightnessPercent(DISPLAY_BRIGHTNESS_DEFAULT_PERCENT);
    } else {
        digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
        backlightPwmReady = false;
    }
#endif

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

bool displayBrightnessSupported() {
    return backlightPwmReady;
}

uint8_t displayGetBrightnessPercent() {
    return backlightPercent;
}

bool displaySetBrightnessPercent(uint8_t percent) {
    if (percent > 100) {
        percent = 100;
    }
    backlightPercent = percent;

#ifdef TFT_BL
    if (!backlightPwmReady) {
        return false;
    }
    ledcWrite(DISPLAY_BRIGHTNESS_PWM_CHANNEL, brightnessPercentToDuty(percent));
    return true;
#else
    return false;
#endif
}
