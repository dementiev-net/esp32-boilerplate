#include "battery.h"

#include "../../config.h"
#include "../board/board_profile.h"
#if defined(ARDUINO_USB_MODE) && (ARDUINO_USB_MODE == 1)
  #include "soc/usb_serial_jtag_struct.h"
#endif

static bool initialized = false;
static bool supported = false;
static int8_t adcPin = -1;

static int pinMillivoltsToBatteryMillivolts(int pinMillivolts) {
    if (pinMillivolts < 0) {
        return -1;
    }

    const long numerator =
        static_cast<long>(pinMillivolts)
        * BATTERY_DIVIDER_NUM
        * BATTERY_CALIBRATION_NUM;
    const long denominator =
        static_cast<long>(BATTERY_DIVIDER_DEN)
        * BATTERY_CALIBRATION_DEN;
    if (denominator <= 0) {
        return -1;
    }

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

bool batteryUsbHostConnected() {
#if defined(ARDUINO_USB_MODE) && (ARDUINO_USB_MODE == 1) && defined(ARDUINO_USB_CDC_ON_BOOT) && (ARDUINO_USB_CDC_ON_BOOT == 1)
    // Для USB Serial JTAG надёжнее отслеживать приходы SOF-фреймов от USB-хоста.
    // Если frame index меняется, считаем, что подключён USB-хост (ПК).
    static bool initialized = false;
    static uint16_t lastFrameIndex = 0;
    static unsigned long lastSofSeenMs = 0;
    static constexpr unsigned long kUsbHostGraceMs = 120;

    const unsigned long now = millis();
    const uint16_t frameIndex = static_cast<uint16_t>(USB_SERIAL_JTAG.fram_num.sof_frame_index);
    if (!initialized) {
        initialized = true;
        lastFrameIndex = frameIndex;
        lastSofSeenMs = 0;
        return false;
    }

    if (frameIndex != lastFrameIndex) {
        lastFrameIndex = frameIndex;
        lastSofSeenMs = now;
    }

    return lastSofSeenMs > 0 && (now - lastSofSeenMs) <= kUsbHostGraceMs;
#elif defined(ARDUINO_USB_CDC_ON_BOOT) && (ARDUINO_USB_CDC_ON_BOOT == 1)
    return static_cast<bool>(Serial);
#else
    return false;
#endif
}

int batteryReadMillivolts() {
    if (!initialized || !supported) {
        return -1;
    }

    static constexpr uint8_t kSamples = 8;
    long pinMilliVoltsSum = 0;
    uint8_t validSamples = 0;

    for (uint8_t i = 0; i < kSamples; i++) {
        int pinMilliVolts = analogReadMilliVolts(static_cast<uint8_t>(adcPin));
        if (pinMilliVolts <= 0) {
            const int raw = analogRead(adcPin);
            if (raw < 0) {
                continue;
            }
            // Fallback для окружений/SDK, где analogReadMilliVolts может вернуть 0.
            pinMilliVolts = static_cast<int>((static_cast<long>(raw) * 3300L) / 4095L);
        }

        if (pinMilliVolts > 0) {
            pinMilliVoltsSum += pinMilliVolts;
            validSamples++;
        }
    }

    if (validSamples == 0) {
        return -1;
    }

    const int pinMilliVoltsAvg = static_cast<int>(pinMilliVoltsSum / validSamples);
    const int millivolts = pinMillivoltsToBatteryMillivolts(pinMilliVoltsAvg);
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
