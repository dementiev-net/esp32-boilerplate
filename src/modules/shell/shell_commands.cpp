#include "shell_commands.h"

#include "../../config.h"
#include "../ble/ble.h"
#include "../board/board_profile.h"
#include "../display/display.h"
#include "../net/net_http_demo.h"
#include "../ota/ota.h"
#include "../power/sleep.h"
#include "../storage/storage.h"
#include "../system/reliability.h"
#include "../wifi/wifi.h"

namespace {

const char* normalizeTag(const char* sourceTag) {
    return (sourceTag != nullptr && sourceTag[0] != '\0') ? sourceTag : "shell";
}

void printHelp(Print& out) {
    out.println("Commands:");
    out.println("  help                     - список команд");
    out.println("  version                  - версия приложения");
    out.println("  status                   - общий статус runtime");
    out.println("  wifi                     - статус Wi-Fi");
    out.println("  net                      - статус HTTP JSON demo");
    out.println("  brightness [0..100]      - статус/установка яркости подсветки");
    out.println("  storage                  - статус файлового backend");
    out.println("  cat <path>               - прочитать файл");
    out.println("  write <path> <text>      - перезаписать файл");
    out.println("  append <path> <text>     - дописать в файл");
    out.println("  heap                     - свободная heap память");
    out.println("  uptime                   - uptime в секундах");
    out.println("  reboot                   - перезагрузка контроллера");
    out.println("  sleep                    - переход в deep sleep");
}

void printWifiStatus(Print& out) {
    out.printf(
        "[wifi] mode=%s connected=%s ssid=%s ip=%s\n",
        wifiGetModeLabel(),
        wifiIsConnected() ? "yes" : "no",
        wifiGetSSID().c_str(),
        wifiGetIP().c_str()
    );
}

void printStorageStatus(Print& out) {
    out.printf(
        "[storage] backend=%s available=%s sd_supported=%s sd_available=%s nvs=%s\n",
        storageFileBackendLabel(),
        storageFileAvailable() ? "yes" : "no",
        sdSupported() ? "yes" : "no",
        sdAvailable() ? "yes" : "no",
        storageNvsAvailable() ? "yes" : "no"
    );
}

void printNetStatus(Print& out) {
    const unsigned long nowMs = millis();
    const unsigned long lastAttempt = netDemoLastAttemptMs();
    const unsigned long lastSuccess = netDemoLastSuccessMs();
    const unsigned long ageAttempt = lastAttempt == 0 ? 0 : (nowMs - lastAttempt) / 1000UL;
    const unsigned long ageSuccess = lastSuccess == 0 ? 0 : (nowMs - lastSuccess) / 1000UL;

    out.printf(
        "[net] enabled=%s ok=%s value=%s ui=%s last_attempt=%lus last_success=%lus\n",
        netDemoIsEnabled() ? "yes" : "no",
        netDemoLastRequestOk() ? "yes" : "no",
        netDemoGetValue().c_str(),
        netDemoGetUiText().c_str(),
        ageAttempt,
        ageSuccess
    );
}

void printBrightnessStatus(Print& out) {
    if (!displayBrightnessSupported()) {
        out.println("[display] brightness control unavailable");
        return;
    }
    out.printf("[display] brightness=%u%%\n", static_cast<unsigned>(displayGetBrightnessPercent()));
}

void printRuntimeStatus(Print& out) {
    const BoardProfile& board = boardGetProfile();
    out.printf("[app] %s v%s\n", APP_NAME, APP_VERSION);
    out.printf("[board] %s\n", board.label);
    out.printf(
        "[features] ota=%u ble=%u littlefs=%u usb_shell=%u telnet_shell=%u net_http=%u self_test=%u\n",
        static_cast<unsigned>(FEATURE_OTA),
        static_cast<unsigned>(FEATURE_BLE),
        static_cast<unsigned>(FEATURE_LITTLEFS),
        static_cast<unsigned>(FEATURE_USB_SHELL),
        static_cast<unsigned>(FEATURE_TELNET_SHELL),
        static_cast<unsigned>(FEATURE_NET_HTTP),
        static_cast<unsigned>(FEATURE_SELF_TEST)
    );
    out.printf(
        "[system] uptime=%lu s heap_free=%u bytes reset=%s(%d) wake=%s wake_pin=%d wdt=%s\n",
        reliabilityUptimeSeconds(),
        static_cast<unsigned>(ESP.getFreeHeap()),
        reliabilityResetReasonLabel(),
        reliabilityResetReasonCode(),
        sleepWakeReasonLabel(),
        sleepWakeButtonPin(),
        reliabilityWatchdogEnabled() ? "on" : "off"
    );
    printWifiStatus(out);
    printNetStatus(out);
    printBrightnessStatus(out);
    printStorageStatus(out);
    out.printf(
        "[wireless] ota_ready=%s ble_ready=%s ble_client=%s\n",
        otaIsReady() ? "yes" : "no",
        bleIsReady() ? "yes" : "no",
        bleIsConnected() ? "yes" : "no"
    );
}

bool splitHeadTail(const String& input, String& head, String& tail) {
    String trimmed = input;
    trimmed.trim();
    if (trimmed.length() == 0) {
        return false;
    }

    const int separator = trimmed.indexOf(' ');
    if (separator < 0) {
        head = trimmed;
        tail = "";
        return true;
    }

    head = trimmed.substring(0, separator);
    tail = trimmed.substring(separator + 1);
    tail.trim();
    return true;
}

bool splitPathAndText(const String& input, String& path, String& text) {
    String trimmed = input;
    trimmed.trim();
    if (trimmed.length() == 0) {
        return false;
    }

    const int separator = trimmed.indexOf(' ');
    if (separator < 0) {
        path = trimmed;
        text = "";
        return false;
    }

    path = trimmed.substring(0, separator);
    text = trimmed.substring(separator + 1);
    text.trim();
    return path.length() > 0;
}

bool parsePercentArg(const String& input, uint8_t& outPercent) {
    String trimmed = input;
    trimmed.trim();
    if (trimmed.length() == 0) {
        return false;
    }

    for (size_t i = 0; i < trimmed.length(); i++) {
        if (!isDigit(trimmed[i])) {
            return false;
        }
    }

    const long parsed = trimmed.toInt();
    if (parsed < 0 || parsed > 100) {
        return false;
    }

    outPercent = static_cast<uint8_t>(parsed);
    return true;
}

void handleBrightness(const String& args, Print& out, const char* sourceTag) {
    String trimmed = args;
    trimmed.trim();
    if (trimmed.length() == 0) {
        printBrightnessStatus(out);
        return;
    }

    uint8_t percent = 0;
    if (!parsePercentArg(trimmed, percent)) {
        out.printf("[%s] usage: brightness <0..100>\n", sourceTag);
        return;
    }

    if (!displaySetBrightnessPercent(percent)) {
        out.println("[display] brightness control unavailable");
        return;
    }

    out.printf("[display] brightness=%u%%\n", static_cast<unsigned>(displayGetBrightnessPercent()));
}

void handleCat(const String& args, Print& out, const char* sourceTag) {
    String path = args;
    path.trim();
    if (path.length() == 0) {
        out.printf("[%s] usage: cat <path>\n", sourceTag);
        return;
    }

    String content = sdReadFile(path.c_str());
    if (content.length() == 0) {
        out.printf("[%s] file is empty or unavailable: %s\n", sourceTag, path.c_str());
        return;
    }

    out.printf("[%s] %s (%u bytes):\n", sourceTag, path.c_str(), static_cast<unsigned>(content.length()));
    out.println(content);
}

void handleWrite(const String& args, bool append, Print& out, const char* sourceTag) {
    String path;
    String text;
    if (!splitPathAndText(args, path, text)) {
        out.printf(
            append ? "[%s] usage: append <path> <text>\n" : "[%s] usage: write <path> <text>\n",
            sourceTag
        );
        return;
    }

    const bool ok = append
        ? sdAppendFile(path.c_str(), text.c_str())
        : sdWriteFile(path.c_str(), text.c_str());

    out.printf(
        "[%s] %s %s: %s\n",
        sourceTag,
        append ? "append" : "write",
        ok ? "ok" : "failed",
        path.c_str()
    );
}

}  // namespace

void shellExecuteCommand(const String& rawLine, Print& out, const char* sourceTag) {
    const char* tag = normalizeTag(sourceTag);
    String command;
    String args;
    if (!splitHeadTail(rawLine, command, args)) {
        return;
    }

    String cmdLower = command;
    cmdLower.toLowerCase();

    if (cmdLower == "help") {
        printHelp(out);
        return;
    }
    if (cmdLower == "version") {
        out.printf("%s v%s\n", APP_NAME, APP_VERSION);
        return;
    }
    if (cmdLower == "status") {
        printRuntimeStatus(out);
        return;
    }
    if (cmdLower == "wifi") {
        printWifiStatus(out);
        return;
    }
    if (cmdLower == "net") {
        printNetStatus(out);
        return;
    }
    if (cmdLower == "brightness") {
        handleBrightness(args, out, tag);
        return;
    }
    if (cmdLower == "storage") {
        printStorageStatus(out);
        return;
    }
    if (cmdLower == "cat") {
        handleCat(args, out, tag);
        return;
    }
    if (cmdLower == "write") {
        handleWrite(args, false, out, tag);
        return;
    }
    if (cmdLower == "append") {
        handleWrite(args, true, out, tag);
        return;
    }
    if (cmdLower == "heap") {
        out.printf("[system] heap_free=%u bytes\n", static_cast<unsigned>(ESP.getFreeHeap()));
        return;
    }
    if (cmdLower == "uptime") {
        out.printf("[system] uptime=%lu s\n", reliabilityUptimeSeconds());
        return;
    }
    if (cmdLower == "reboot") {
        out.printf("[%s] rebooting...\n", tag);
        delay(80);
        ESP.restart();
        return;
    }
    if (cmdLower == "sleep") {
        out.printf("[%s] entering deep sleep...\n", tag);
        sleepEnterDeepSleep();
        return;
    }

    out.printf("[%s] unknown command: %s\n", tag, command.c_str());
}

