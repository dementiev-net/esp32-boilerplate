#include "sleep.h"

#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>

#include "../board/board_profile.h"

static bool initialized = false;
static bool wakeByButtonSupported = false;
static int wakeButtonGpio = -1;
static const char* wakeReason = "unknown";

static bool isRtcPinForEsp32S3(int gpio) {
    return gpio >= 0 && gpio <= 21;
}

static const char* mapWakeReason(esp_sleep_wakeup_cause_t cause) {
    switch (cause) {
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            return "cold_boot";
        case ESP_SLEEP_WAKEUP_EXT1:
            return "button_wakeup";
        case ESP_SLEEP_WAKEUP_TIMER:
            return "timer_wakeup";
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            return "touch_wakeup";
        case ESP_SLEEP_WAKEUP_ULP:
            return "ulp_wakeup";
        case ESP_SLEEP_WAKEUP_GPIO:
            return "gpio_wakeup";
        case ESP_SLEEP_WAKEUP_UART:
            return "uart_wakeup";
        default:
            return "other_wakeup";
    }
}

void sleepInit() {
    const BoardProfile& board = boardGetProfile();
    wakeReason = mapWakeReason(esp_sleep_get_wakeup_cause());

    wakeButtonGpio = static_cast<int>(board.pins.buttonTop);
    wakeByButtonSupported = false;

    if (!isRtcPinForEsp32S3(wakeButtonGpio)) {
        initialized = true;
        return;
    }

    const uint64_t wakeMask = 1ULL << wakeButtonGpio;
    const esp_err_t wakeErr = esp_sleep_enable_ext1_wakeup(wakeMask, ESP_EXT1_WAKEUP_ANY_LOW);
    if (wakeErr != ESP_OK) {
        initialized = true;
        return;
    }

    rtc_gpio_pullup_en(static_cast<gpio_num_t>(wakeButtonGpio));
    rtc_gpio_pulldown_dis(static_cast<gpio_num_t>(wakeButtonGpio));
    wakeByButtonSupported = true;
    initialized = true;
}

bool sleepCanWakeByButton() {
    return wakeByButtonSupported;
}

int sleepWakeButtonPin() {
    return wakeByButtonSupported ? wakeButtonGpio : -1;
}

const char* sleepWakeReasonLabel() {
    return wakeReason;
}

void sleepEnterDeepSleep() {
    if (!initialized) {
        sleepInit();
    }

    if (wakeByButtonSupported) {
        Serial.printf("[Power] Deep sleep. Wake on TOP button GPIO%d\n", wakeButtonGpio);
    } else {
        Serial.println("[Power] Deep sleep. Wake source is not configured");
    }
    delay(80);
    esp_deep_sleep_start();
}
