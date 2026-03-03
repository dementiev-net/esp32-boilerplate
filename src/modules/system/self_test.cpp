#include "self_test.h"

#include <cstring>

#include "../board/board_profile.h"
#include "../buttons/buttons.h"
#include "../display/display.h"
#include "../power/battery.h"
#include "../storage/storage.h"
#include "../wifi/wifi.h"

namespace {

const char* okLabel(bool value) {
    return value ? "OK" : "FAIL";
}

const char* batteryLabel(const SelfTestResult& result) {
    if (!result.batterySupported) {
        return "N/A";
    }
    if (result.usbHostConnected) {
        return "USB";
    }
    if (result.batteryPercent < 0) {
        return "N/A";
    }
    if (!result.batteryOk) {
        return "FAIL";
    }
    return "OK";
}

}  // namespace

SelfTestResult selfTestRun() {
    SelfTestResult result;

    const BoardProfile& board = boardGetProfile();
    result.displayOk = board.display.width > 0 && board.display.height > 0;

    result.topPressed = buttonsIsTopPressed();
    result.bottomPressed = buttonsIsBottomPressed();
    result.buttonsOk = true;

    result.wifiConnected = wifiIsConnected();
    const char* modeLabel = wifiGetModeLabel();
    if (strcmp(modeLabel, "STA") == 0 || strcmp(modeLabel, "AP") == 0) {
        result.wifiOk = result.wifiConnected;
    } else {
        // AUTO в self-test трактуем как "стек Wi-Fi поднят", даже если роутер не настроен.
        result.wifiOk = strcmp(modeLabel, "AUTO") == 0;
    }

    result.storageNvsOk = storageNvsAvailable();
    result.storageFileOk = storageFileAvailable();
    result.storageOk = result.storageNvsOk && result.storageFileOk;

    result.batterySupported = batteryIsSupported();
    result.usbHostConnected = batteryUsbHostConnected();
    if (!result.batterySupported) {
        result.batteryOk = true;
        result.batteryMillivolts = -1;
        result.batteryPercent = -1;
    } else if (result.usbHostConnected) {
        result.batteryMillivolts = batteryReadMillivolts();
        result.batteryPercent = batteryMillivoltsToPercent(result.batteryMillivolts);
        result.batteryOk = true;
    } else {
        static constexpr uint8_t kBatteryProbeAttempts = 3;
        result.batteryMillivolts = -1;
        for (uint8_t attempt = 0; attempt < kBatteryProbeAttempts; attempt++) {
            result.batteryMillivolts = batteryReadMillivolts();
            if (result.batteryMillivolts >= 0) {
                break;
            }
            delay(20);
        }
        result.batteryPercent = batteryMillivoltsToPercent(result.batteryMillivolts);
        result.batteryOk = result.batteryMillivolts >= 0;
    }

    const bool batteryCheckPassed =
        !result.batterySupported
        || result.usbHostConnected
        || result.batteryPercent < 0
        || result.batteryOk;

    result.passed = result.displayOk
        && result.buttonsOk
        && result.wifiOk
        && result.storageOk
        && batteryCheckPassed;
    return result;
}

void selfTestPrintReport(const SelfTestResult& result) {
    Serial.println("[SelfTest] Startup diagnostics:");
    Serial.printf(
        "[SelfTest] display=%s buttons=%s wifi=%s (connected=%s) storage=%s (nvs=%s file=%s) battery=%s\n",
        okLabel(result.displayOk),
        okLabel(result.buttonsOk),
        okLabel(result.wifiOk),
        result.wifiConnected ? "yes" : "no",
        okLabel(result.storageOk),
        okLabel(result.storageNvsOk),
        okLabel(result.storageFileOk),
        batteryLabel(result)
    );
    if (!result.batterySupported) {
        Serial.println("[SelfTest] battery=N/A");
    } else if (result.usbHostConnected) {
        if (result.batteryPercent >= 0) {
            Serial.printf(
                "[SelfTest] battery=USB (%d%%, %dmV)\n",
                result.batteryPercent,
                result.batteryMillivolts
            );
        } else {
            Serial.println("[SelfTest] battery=USB");
        }
    } else if (result.batteryPercent >= 0) {
        Serial.printf(
            "[SelfTest] battery=%d%% (%dmV)\n",
            result.batteryPercent,
            result.batteryMillivolts
        );
    } else {
        Serial.println("[SelfTest] battery=N/A (not detected)");
    }
    Serial.printf(
        "[SelfTest] buttons_state top=%d bottom=%d\n",
        result.topPressed ? 1 : 0,
        result.bottomPressed ? 1 : 0
    );
    Serial.printf("[SelfTest] RESULT: %s\n", result.passed ? "PASS" : "FAIL");
}

void selfTestRenderReport(const SelfTestResult& result, int y) {
    const BoardProfile& board = boardGetProfile();
    const uint16_t okColor = TFT_GREEN;
    const uint16_t failColor = 0xFD20;
    const uint16_t neutralColor = 0x7BEF;

    char line1[24];
    snprintf(line1, sizeof(line1), "SELFTEST:%s", result.passed ? "PASS" : "FAIL");
    displayFillRect(0, y, board.display.width, 36, TFT_BLACK);
    displayPrint(2, y, line1, result.passed ? okColor : failColor, 1);

    char line2[40];
    char line3[40];
    if (board.display.width >= 200) {
        snprintf(
            line2,
            sizeof(line2),
            "D:%s B:%s W:%s",
            okLabel(result.displayOk),
            okLabel(result.buttonsOk),
            okLabel(result.wifiOk)
        );
        if (!result.batterySupported || result.batteryPercent < 0) {
            snprintf(
                line3,
                sizeof(line3),
                "NVS:%s FS:%s BAT:N/A",
                okLabel(result.storageNvsOk),
                okLabel(result.storageFileOk)
            );
        } else if (result.usbHostConnected) {
            snprintf(
                line3,
                sizeof(line3),
                "NVS:%s FS:%s BAT:USB",
                okLabel(result.storageNvsOk),
                okLabel(result.storageFileOk)
            );
        } else if (result.batteryOk) {
            snprintf(
                line3,
                sizeof(line3),
                "NVS:%s FS:%s BAT:%d%%",
                okLabel(result.storageNvsOk),
                okLabel(result.storageFileOk),
                result.batteryPercent
            );
        } else {
            snprintf(
                line3,
                sizeof(line3),
                "NVS:%s FS:%s BAT:FAIL",
                okLabel(result.storageNvsOk),
                okLabel(result.storageFileOk)
            );
        }
    } else {
        snprintf(
            line2,
            sizeof(line2),
            "D:%s B:%s W:%s",
            okLabel(result.displayOk),
            okLabel(result.buttonsOk),
            okLabel(result.wifiOk)
        );
        if (!result.batterySupported || result.batteryPercent < 0) {
            snprintf(
                line3,
                sizeof(line3),
                "N:%s F:%s BA:N/A",
                okLabel(result.storageNvsOk),
                okLabel(result.storageFileOk)
            );
        } else if (result.usbHostConnected) {
            snprintf(
                line3,
                sizeof(line3),
                "N:%s F:%s BA:USB",
                okLabel(result.storageNvsOk),
                okLabel(result.storageFileOk)
            );
        } else if (result.batteryOk) {
            snprintf(
                line3,
                sizeof(line3),
                "N:%s F:%s BA:%d%%",
                okLabel(result.storageNvsOk),
                okLabel(result.storageFileOk),
                result.batteryPercent
            );
        } else {
            snprintf(
                line3,
                sizeof(line3),
                "N:%s F:%s BA:FAIL",
                okLabel(result.storageNvsOk),
                okLabel(result.storageFileOk)
            );
        }
    }

    displayPrint(2, y + 12, line2, neutralColor, 1);
    displayPrint(2, y + 24, line3, neutralColor, 1);
}
