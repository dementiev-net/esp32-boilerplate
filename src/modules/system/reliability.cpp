#include "reliability.h"

#include <Arduino.h>
#include <esp_system.h>
#include <esp_task_wdt.h>

#include "../../config.h"

namespace {

bool initialized = false;
bool watchdogEnabled = false;
unsigned long lastWatchdogFeedMs = 0;
int resetReasonCode = static_cast<int>(ESP_RST_UNKNOWN);
const char* resetReasonLabel = "unknown";

const char* mapResetReason(esp_reset_reason_t reason) {
    switch (reason) {
        case ESP_RST_UNKNOWN:
            return "unknown";
        case ESP_RST_POWERON:
            return "power_on";
        case ESP_RST_EXT:
            return "external";
        case ESP_RST_SW:
            return "software";
        case ESP_RST_PANIC:
            return "panic";
        case ESP_RST_INT_WDT:
            return "int_wdt";
        case ESP_RST_TASK_WDT:
            return "task_wdt";
        case ESP_RST_WDT:
            return "other_wdt";
        case ESP_RST_DEEPSLEEP:
            return "deep_sleep";
        case ESP_RST_BROWNOUT:
            return "brownout";
        case ESP_RST_SDIO:
            return "sdio";
        default:
            return "other";
    }
}

} // namespace

void reliabilityInit() {
    const esp_reset_reason_t reason = esp_reset_reason();
    resetReasonCode = static_cast<int>(reason);
    resetReasonLabel = mapResetReason(reason);

#if FEATURE_WATCHDOG
    const esp_err_t initErr = esp_task_wdt_init(WATCHDOG_TIMEOUT_SEC, true);
    if (initErr != ESP_OK && initErr != ESP_ERR_INVALID_STATE) {
        Serial.printf("[WDT] init failed: err=%d\n", static_cast<int>(initErr));
        watchdogEnabled = false;
    } else {
        const esp_err_t addErr = esp_task_wdt_add(nullptr);
        if (addErr != ESP_OK && addErr != ESP_ERR_INVALID_STATE) {
            Serial.printf("[WDT] add current task failed: err=%d\n", static_cast<int>(addErr));
            watchdogEnabled = false;
        } else {
            watchdogEnabled = true;
            lastWatchdogFeedMs = millis();
            Serial.printf("[WDT] enabled timeout=%us\n", static_cast<unsigned>(WATCHDOG_TIMEOUT_SEC));
        }
    }
#else
    watchdogEnabled = false;
    Serial.println("[WDT] disabled by build profile");
#endif

    initialized = true;
}

void reliabilityLoop() {
    if (!initialized || !watchdogEnabled) {
        return;
    }

    const unsigned long nowMs = millis();
    if (nowMs - lastWatchdogFeedMs < WATCHDOG_FEED_INTERVAL_MS) {
        return;
    }

    const esp_err_t resetErr = esp_task_wdt_reset();
    if (resetErr == ESP_OK) {
        lastWatchdogFeedMs = nowMs;
    } else {
        Serial.printf("[WDT] feed failed: err=%d\n", static_cast<int>(resetErr));
    }
}

bool reliabilityWatchdogEnabled() {
    return watchdogEnabled;
}

const char* reliabilityResetReasonLabel() {
    return resetReasonLabel;
}

int reliabilityResetReasonCode() {
    return resetReasonCode;
}

unsigned long reliabilityUptimeSeconds() {
    return millis() / 1000UL;
}
