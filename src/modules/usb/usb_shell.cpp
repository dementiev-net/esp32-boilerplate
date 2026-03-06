#include "usb_shell.h"

#include <Arduino.h>

#include "../../config.h"
#if FEATURE_USB_SHELL
#include "../shell/shell_commands.h"

static bool initialized = false;
static String lineBuffer = "";
static bool previousWasCarriageReturn = false;
static const size_t kMaxLineLength = 200;

static void printPrompt() {
    Serial.print("usb> ");
}

void usbShellInit() {
    if (initialized) {
        return;
    }

    initialized = true;
    lineBuffer = "";
    previousWasCarriageReturn = false;
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
            shellExecuteCommand(lineBuffer, Serial, "usb");
            lineBuffer = "";
            printPrompt();
            previousWasCarriageReturn = true;
            continue;
        }

        if (c == '\n') {
            if (previousWasCarriageReturn) {
                previousWasCarriageReturn = false;
                continue;
            }
            shellExecuteCommand(lineBuffer, Serial, "usb");
            lineBuffer = "";
            printPrompt();
            continue;
        }

        previousWasCarriageReturn = false;

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
