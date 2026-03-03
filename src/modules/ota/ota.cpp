#include "ota.h"

#include <Arduino.h>
#include "../../config.h"
#if FEATURE_OTA
#include <ArduinoOTA.h>

#include "../board/board_profile.h"
#include "../wifi/wifi.h"

static bool initialized = false;
static bool started = false;

static String buildOtaHostname() {
    const BoardProfile& board = boardGetProfile();
    const char* suffix = board.id == BoardId::TDisplayS3 ? "tdisplay-s3" : "tqt-pro";

    String hostname = OTA_HOSTNAME_PREFIX;
    hostname += "-";
    hostname += suffix;
    return hostname;
}

void otaInit() {
    if (initialized) {
        return;
    }

    ArduinoOTA.onStart([]() {
        const char* type = ArduinoOTA.getCommand() == U_FLASH ? "firmware" : "filesystem";
        Serial.printf("[OTA] Start update (%s)\n", type);
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("\n[OTA] Update complete, rebooting...");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        const unsigned int percent = total == 0 ? 0 : (progress * 100U) / total;
        Serial.printf("\r[OTA] Progress: %u%%", percent);
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("\n[OTA] Error[%u]: ", static_cast<unsigned>(error));
        switch (error) {
            case OTA_AUTH_ERROR:
                Serial.println("Auth failed");
                break;
            case OTA_BEGIN_ERROR:
                Serial.println("Begin failed");
                break;
            case OTA_CONNECT_ERROR:
                Serial.println("Connect failed");
                break;
            case OTA_RECEIVE_ERROR:
                Serial.println("Receive failed");
                break;
            case OTA_END_ERROR:
                Serial.println("End failed");
                break;
            default:
                Serial.println("Unknown");
                break;
        }
    });

    initialized = true;
}

void otaLoop() {
    if (!initialized) {
        return;
    }

    if (!started) {
        if (!wifiIsConnected()) {
            return;
        }

        const String hostname = buildOtaHostname();
        ArduinoOTA.setHostname(hostname.c_str());
        ArduinoOTA.setPort(OTA_PORT);
        ArduinoOTA.setPassword(OTA_PASSWORD);
        ArduinoOTA.begin();
        started = true;

        Serial.printf(
            "[OTA] Ready: host=%s ip=%s port=%u\n",
            hostname.c_str(),
            wifiGetIP().c_str(),
            static_cast<unsigned>(OTA_PORT)
        );
    }

    if (wifiIsConnected()) {
        ArduinoOTA.handle();
    }
}

bool otaIsReady() {
    return started;
}
#else

void otaInit() {}

void otaLoop() {}

bool otaIsReady() {
    return false;
}
#endif
