#include "usb_shell.h"

#include <Arduino.h>

#include "../../config.h"
#if FEATURE_USB_SHELL
#include "../ble/ble.h"
#include "../board/board_profile.h"
#include "../ota/ota.h"
#include "../power/sleep.h"
#include "../storage/storage.h"
#include "../wifi/wifi.h"

static bool initialized = false;
static String lineBuffer = "";
static const size_t kMaxLineLength = 200;

static void printPrompt() {
    Serial.print("usb> ");
}

static void printHelp() {
    Serial.println("Commands:");
    Serial.println("  help                     - список команд");
    Serial.println("  version                  - версия приложения");
    Serial.println("  status                   - общий статус runtime");
    Serial.println("  wifi                     - статус Wi-Fi");
    Serial.println("  storage                  - статус файлового backend");
    Serial.println("  cat <path>               - прочитать файл");
    Serial.println("  write <path> <text>      - перезаписать файл");
    Serial.println("  append <path> <text>     - дописать в файл");
    Serial.println("  heap                     - свободная heap память");
    Serial.println("  uptime                   - uptime в секундах");
    Serial.println("  reboot                   - перезагрузка контроллера");
    Serial.println("  sleep                    - переход в deep sleep");
}

static void printWifiStatus() {
    Serial.printf(
        "[wifi] mode=%s connected=%s ssid=%s ip=%s\n",
        wifiGetModeLabel(),
        wifiIsConnected() ? "yes" : "no",
        wifiGetSSID().c_str(),
        wifiGetIP().c_str()
    );
}

static void printStorageStatus() {
    Serial.printf(
        "[storage] backend=%s available=%s sd_supported=%s sd_available=%s nvs=%s\n",
        storageFileBackendLabel(),
        storageFileAvailable() ? "yes" : "no",
        sdSupported() ? "yes" : "no",
        sdAvailable() ? "yes" : "no",
        storageNvsAvailable() ? "yes" : "no"
    );
}

static void printRuntimeStatus() {
    const BoardProfile& board = boardGetProfile();
    Serial.printf("[app] %s v%s\n", APP_NAME, APP_VERSION);
    Serial.printf("[board] %s\n", board.label);
    Serial.printf(
        "[features] ota=%u ble=%u littlefs=%u usb_shell=%u\n",
        static_cast<unsigned>(FEATURE_OTA),
        static_cast<unsigned>(FEATURE_BLE),
        static_cast<unsigned>(FEATURE_LITTLEFS),
        static_cast<unsigned>(FEATURE_USB_SHELL)
    );
    Serial.printf(
        "[system] uptime=%lu s heap_free=%u bytes wake=%s wake_pin=%d\n",
        millis() / 1000UL,
        static_cast<unsigned>(ESP.getFreeHeap()),
        sleepWakeReasonLabel(),
        sleepWakeButtonPin()
    );
    printWifiStatus();
    printStorageStatus();
    Serial.printf(
        "[wireless] ota_ready=%s ble_ready=%s ble_client=%s\n",
        otaIsReady() ? "yes" : "no",
        bleIsReady() ? "yes" : "no",
        bleIsConnected() ? "yes" : "no"
    );
}

static bool splitHeadTail(const String& input, String& head, String& tail) {
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

static bool splitPathAndText(const String& input, String& path, String& text) {
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

static void handleCat(const String& args) {
    String path = args;
    path.trim();
    if (path.length() == 0) {
        Serial.println("[usb] usage: cat <path>");
        return;
    }

    String content = sdReadFile(path.c_str());
    if (content.length() == 0) {
        Serial.printf("[usb] file is empty or unavailable: %s\n", path.c_str());
        return;
    }

    Serial.printf("[usb] %s (%u bytes):\n", path.c_str(), static_cast<unsigned>(content.length()));
    Serial.println(content);
}

static void handleWrite(const String& args, bool append) {
    String path;
    String text;
    if (!splitPathAndText(args, path, text)) {
        Serial.println(append ? "[usb] usage: append <path> <text>" : "[usb] usage: write <path> <text>");
        return;
    }

    const bool ok = append
        ? sdAppendFile(path.c_str(), text.c_str())
        : sdWriteFile(path.c_str(), text.c_str());

    Serial.printf(
        "[usb] %s %s: %s\n",
        append ? "append" : "write",
        ok ? "ok" : "failed",
        path.c_str()
    );
}

static void executeCommand(const String& rawLine) {
    String command;
    String args;
    if (!splitHeadTail(rawLine, command, args)) {
        return;
    }

    String cmdLower = command;
    cmdLower.toLowerCase();

    if (cmdLower == "help") {
        printHelp();
        return;
    }
    if (cmdLower == "version") {
        Serial.printf("%s v%s\n", APP_NAME, APP_VERSION);
        return;
    }
    if (cmdLower == "status") {
        printRuntimeStatus();
        return;
    }
    if (cmdLower == "wifi") {
        printWifiStatus();
        return;
    }
    if (cmdLower == "storage") {
        printStorageStatus();
        return;
    }
    if (cmdLower == "cat") {
        handleCat(args);
        return;
    }
    if (cmdLower == "write") {
        handleWrite(args, false);
        return;
    }
    if (cmdLower == "append") {
        handleWrite(args, true);
        return;
    }
    if (cmdLower == "heap") {
        Serial.printf("[system] heap_free=%u bytes\n", static_cast<unsigned>(ESP.getFreeHeap()));
        return;
    }
    if (cmdLower == "uptime") {
        Serial.printf("[system] uptime=%lu s\n", millis() / 1000UL);
        return;
    }
    if (cmdLower == "reboot") {
        Serial.println("[usb] rebooting...");
        delay(80);
        ESP.restart();
        return;
    }
    if (cmdLower == "sleep") {
        Serial.println("[usb] entering deep sleep...");
        sleepEnterDeepSleep();
        return;
    }

    Serial.printf("[usb] unknown command: %s\n", command.c_str());
}

void usbShellInit() {
    if (initialized) {
        return;
    }

    initialized = true;
    lineBuffer = "";
    Serial.println("[USB] CDC shell ready. Type 'help'.");
    printPrompt();
}

void usbShellLoop() {
    if (!initialized) {
        return;
    }

    while (Serial.available() > 0) {
        const char c = static_cast<char>(Serial.read());

        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            executeCommand(lineBuffer);
            lineBuffer = "";
            printPrompt();
            continue;
        }

        if (lineBuffer.length() >= static_cast<int>(kMaxLineLength)) {
            lineBuffer = "";
            Serial.println("[usb] input too long, line dropped");
            printPrompt();
            continue;
        }

        lineBuffer += c;
    }
}
#else

void usbShellInit() {}

void usbShellLoop() {}
#endif

