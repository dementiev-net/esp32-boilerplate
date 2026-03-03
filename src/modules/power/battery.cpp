#include "battery.h"

#include "../../config.h"
#include "../board/board_profile.h"

static bool initialized = false;
static bool supported = false;
static int8_t adcPin = -1;

static int rawToMillivolts(int raw) {
    const long numerator = static_cast<long>(raw) * 3300L * BATTERY_DIVIDER_NUM;
    const long denominator = 4095L * BATTERY_DIVIDER_DEN;
    return static_cast<int>(numerator / denominator);
}

void batteryInit() {
    const BoardProfile& board = boardGetProfile();
    adcPin = board.pins.batteryAdcPin;
    supported = adcPin >= 0;
    initialized = true;

    if (!supported) {
        Serial.println("[Battery] Unsupported on this board profile");
        return;
    }

    pinMode(adcPin, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(static_cast<uint8_t>(adcPin), ADC_11db);

    Serial.printf("[Battery] ADC pin=%d ready\n", adcPin);
}

bool batteryIsSupported() {
    return supported;
}

int batteryReadMillivolts() {
    if (!initialized || !supported) {
        return -1;
    }

    static constexpr uint8_t kSamples = 4;
    long rawSum = 0;

    for (uint8_t i = 0; i < kSamples; i++) {
        const int raw = analogRead(adcPin);
        if (raw < 0) {
            return -1;
        }
        rawSum += raw;
    }

    const int rawAvg = static_cast<int>(rawSum / kSamples);
    const int millivolts = rawToMillivolts(rawAvg);
    if (millivolts < BATTERY_MIN_VALID_MV || millivolts > BATTERY_MAX_VALID_MV) {
        return -1;
    }

    return millivolts;
}

int batteryMillivoltsToPercent(int millivolts) {
    if (millivolts < 0) {
        return -1;
    }

    if (BATTERY_PERCENT_FULL_MV <= BATTERY_PERCENT_EMPTY_MV) {
        return -1;
    }

    if (millivolts <= BATTERY_PERCENT_EMPTY_MV) {
        return 0;
    }
    if (millivolts >= BATTERY_PERCENT_FULL_MV) {
        return 100;
    }

    const long range = static_cast<long>(BATTERY_PERCENT_FULL_MV) - BATTERY_PERCENT_EMPTY_MV;
    const long normalized = static_cast<long>(millivolts) - BATTERY_PERCENT_EMPTY_MV;
    return static_cast<int>((normalized * 100L) / range);
}
