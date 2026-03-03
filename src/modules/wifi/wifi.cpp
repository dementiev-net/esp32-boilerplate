#include "wifi.h"
#include "../../config.h"
#include <WiFi.h>
#include <WiFiManager.h>

static WiFiManager wifiManager;
static bool connected = false;

void wifiInit() {
    Serial.println("[WiFi] Initializing...");

    wifiManager.setConnectTimeout(WIFI_TIMEOUT_MS / 1000);
    wifiManager.setConfigPortalTimeout(180);
    wifiManager.setConfigPortalBlocking(false); // неблокирующий режим

    bool result = wifiManager.autoConnect(WIFI_AP_NAME, WIFI_AP_PASSWORD);

    if (result) {
        connected = true;
        Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        connected = false;
        Serial.println("[WiFi] Starting config portal...");
    }
}

void wifiLoop() {
    wifiManager.process(); // обрабатывает портал в неблокирующем режиме

    if (connected && WiFi.status() != WL_CONNECTED) {
        connected = false;
        Serial.println("[WiFi] Connection lost, reconnecting...");
        WiFi.reconnect();
    }

    if (!connected && WiFi.status() == WL_CONNECTED) {
        connected = true;
        Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    }
}

void wifiReset() {
    wifiManager.resetSettings();
    ESP.restart();
}

bool wifiIsConnected() {
    return connected;
}

String wifiGetIP() {
    return WiFi.localIP().toString();
}

String wifiGetSSID() {
    return WiFi.SSID();
}