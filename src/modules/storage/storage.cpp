#include "storage.h"
#include "../../config.h"
#include <Preferences.h>
#include <SD.h>
#include <SPI.h>

static Preferences prefs;
static bool sdReady = false;

// ===== NVS =====

void storageInit() {
    Serial.println("[Storage] Initializing NVS...");
    prefs.begin(NVS_NAMESPACE, false);
    Serial.println("[Storage] NVS ready.");

#ifdef SD_CS
    Serial.println("[Storage] Initializing SD card...");
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    if (SD.begin(SD_CS)) {
        sdReady = true;
        Serial.println("[Storage] SD card ready.");
    } else {
        Serial.println("[Storage] SD card not found, skipping.");
    }
#endif
}

void storageSetString(const char* key, const char* value) {
    prefs.putString(key, value);
}

String storageGetString(const char* key, const char* defaultValue) {
    return prefs.getString(key, defaultValue);
}

void storageSetInt(const char* key, int value) {
    prefs.putInt(key, value);
}

int storageGetInt(const char* key, int defaultValue) {
    return prefs.getInt(key, defaultValue);
}

// ===== SD =====

bool sdAvailable() {
    return sdReady;
}

String sdReadFile(const char* path) {
    if (!sdReady) return "";
    File file = SD.open(path);
    if (!file) {
        Serial.printf("[Storage] Failed to open file: %s\n", path);
        return "";
    }
    String content = "";
    while (file.available()) {
        content += (char)file.read();
    }
    file.close();
    return content;
}

bool sdWriteFile(const char* path, const char* content) {
    if (!sdReady) return false;
    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        Serial.printf("[Storage] Failed to write file: %s\n", path);
        return false;
    }
    file.print(content);
    file.close();
    return true;
}