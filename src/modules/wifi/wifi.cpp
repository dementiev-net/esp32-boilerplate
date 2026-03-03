#include "wifi.h"
#include "wifi_config.h"
#include "../../config.h"
#include "../storage/storage.h"
#include <WiFi.h>
#include <WiFiManager.h>

static WiFiManager wifiManager;
static bool connected = false;
static WifiConfigMode runtimeMode = WifiConfigMode::AUTO;
static unsigned long lastStaReconnectMs = 0;

static const unsigned long STA_RECONNECT_INTERVAL_MS = 5000;

static bool loadWifiConfigFromFileStorage(WifiConfigData& out) {
    String configRaw = sdReadFile(WIFI_CONFIG_FILE);
    if (configRaw.length() == 0) return false;
    WifiConfigDefaults defaults = { WIFI_AP_NAME, WIFI_AP_PASSWORD };
    return parseWifiConfigText(configRaw.c_str(), defaults, out);
}

static void startAccessPoint(const std::string& ssid, const std::string& password) {
    WiFi.mode(WIFI_AP);
    if (password.size() >= 8) {
        WiFi.softAP(ssid.c_str(), password.c_str());
    } else {
        WiFi.softAP(ssid.c_str());
    }
    connected = true;
    Serial.printf("[WiFi] AP mode: SSID=%s, IP=%s\n", ssid.c_str(), WiFi.softAPIP().toString().c_str());
}

static bool connectStaBlocking(const std::string& ssid, const std::string& password, unsigned long timeoutMs) {
    if (ssid.empty()) {
        Serial.println("[WiFi] STA mode selected, but sta_ssid is empty.");
        connected = false;
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.printf("[WiFi] STA mode: connecting to %s...\n", ssid.c_str());

    const unsigned long startedAt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startedAt < timeoutMs) {
        delay(200);
    }

    connected = (WiFi.status() == WL_CONNECTED);
    if (connected) {
        Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("[WiFi] STA connect timeout.");
    }
    return connected;
}

void wifiInit() {
    Serial.println("[WiFi] Initializing...");

    WifiConfigData fileConfig;
    // Приоритет: явный режим из /wifi.conf (если файл доступен и конфиг валиден).
    if (loadWifiConfigFromFileStorage(fileConfig)) {
        runtimeMode = fileConfig.mode;

        if (runtimeMode == WifiConfigMode::AP) {
            startAccessPoint(fileConfig.apSsid, fileConfig.apPassword);
            return;
        }

        if (runtimeMode == WifiConfigMode::STA) {
            connectStaBlocking(fileConfig.staSsid, fileConfig.staPassword, WIFI_TIMEOUT_MS);
            return;
        }

        Serial.println("[WiFi] Config mode=auto, fallback to WiFiManager.");
    }

    runtimeMode = WifiConfigMode::AUTO;
    // AUTO режим: сохраненные WiFiManager настройки + неблокирующий портал.
    wifiManager.setConnectTimeout(WIFI_TIMEOUT_MS / 1000);
    wifiManager.setConfigPortalTimeout(180);
    wifiManager.setConfigPortalBlocking(false); // неблокирующий режим

    bool result = wifiManager.autoConnect(WIFI_AP_NAME, WIFI_AP_PASSWORD);
    connected = result;
    if (result) {
        Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("[WiFi] Starting config portal...");
    }
}

void wifiLoop() {
    if (runtimeMode == WifiConfigMode::AP) {
        connected = true;
        return;
    }

    if (runtimeMode == WifiConfigMode::STA) {
        if (WiFi.status() == WL_CONNECTED) {
            if (!connected) {
                connected = true;
                Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
            }
            return;
        }

        if (connected) {
            connected = false;
            Serial.println("[WiFi] Connection lost.");
        }

        unsigned long now = millis();
        if (now - lastStaReconnectMs >= STA_RECONNECT_INTERVAL_MS) {
            lastStaReconnectMs = now;
            Serial.println("[WiFi] Reconnecting STA...");
            WiFi.reconnect();
        }
        return;
    }

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

bool wifiIsConnected() {
    return connected;
}

String wifiGetIP() {
    if (runtimeMode == WifiConfigMode::AP) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
}

String wifiGetSSID() {
    if (runtimeMode == WifiConfigMode::AP) {
        return WiFi.softAPSSID();
    }
    return WiFi.SSID();
}

const char* wifiGetModeLabel() {
    switch (runtimeMode) {
        case WifiConfigMode::AP:
            return "AP";
        case WifiConfigMode::STA:
            return "STA";
        case WifiConfigMode::AUTO:
        default:
            return "AUTO";
    }
}

bool wifiIsStaMode() {
    return runtimeMode == WifiConfigMode::STA;
}

bool wifiIsApMode() {
    return runtimeMode == WifiConfigMode::AP;
}
