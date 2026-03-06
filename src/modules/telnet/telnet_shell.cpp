#include "telnet_shell.h"

#include <Arduino.h>

#include "../../config.h"
#if FEATURE_TELNET_SHELL
#include <WiFi.h>

#include "../shell/shell_commands.h"
#include "../wifi/wifi.h"

static bool initialized = false;
static bool serverStarted = false;
static WiFiServer server(TELNET_SHELL_PORT);
static WiFiClient client;
static String lineBuffer = "";
static bool previousWasCarriageReturn = false;

static void printPrompt() {
    if (client && client.connected()) {
        client.print("telnet> ");
    }
}

static void closeClient(const char* reason) {
    if (!client) {
        return;
    }

    if (reason != nullptr && reason[0] != '\0') {
        Serial.printf("[Telnet] Client disconnected: %s\n", reason);
    }
    client.stop();
    lineBuffer = "";
    previousWasCarriageReturn = false;
}

static void acceptClientIfPending() {
    WiFiClient incoming = server.available();
    if (!incoming) {
        return;
    }

    if (client && client.connected()) {
        incoming.println("[telnet] busy");
        incoming.stop();
        return;
    }

    client = incoming;
    client.setNoDelay(true);
    lineBuffer = "";
    previousWasCarriageReturn = false;

    Serial.printf("[Telnet] Client connected: %s\n", client.remoteIP().toString().c_str());
    client.println("[Telnet] Shell ready. Type 'help'.");
    printPrompt();
}

void telnetShellInit() {
    if (initialized) {
        return;
    }

    initialized = true;
    serverStarted = false;
    lineBuffer = "";
    previousWasCarriageReturn = false;
}

void telnetShellLoop() {
    if (!initialized) {
        return;
    }

    if (!wifiIsConnected()) {
        if (client && client.connected()) {
            closeClient("wifi down");
        }
        return;
    }

    if (!serverStarted) {
        server.begin();
        server.setNoDelay(true);
        serverStarted = true;
        Serial.printf("[Telnet] Ready: ip=%s port=%u\n", wifiGetIP().c_str(), static_cast<unsigned>(TELNET_SHELL_PORT));
    }

    if (client && !client.connected()) {
        closeClient("remote closed");
    }

    acceptClientIfPending();

    if (!client || !client.connected()) {
        return;
    }

    while (client.available() > 0) {
        const char c = static_cast<char>(client.read());

        if (c == '\r') {
            shellExecuteCommand(lineBuffer, client, "telnet");
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
            shellExecuteCommand(lineBuffer, client, "telnet");
            lineBuffer = "";
            printPrompt();
            continue;
        }

        previousWasCarriageReturn = false;

        if (lineBuffer.length() >= static_cast<int>(TELNET_SHELL_MAX_LINE_LEN)) {
            lineBuffer = "";
            client.println("[telnet] input too long, line dropped");
            printPrompt();
            continue;
        }

        lineBuffer += c;
    }
}
#else

void telnetShellInit() {}

void telnetShellLoop() {}
#endif

