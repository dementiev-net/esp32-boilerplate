#include "storage.h"
#include "storage_io.h"
#include "../../config.h"
#include "../board/board_profile.h"
#include <FS.h>
#if FEATURE_LITTLEFS
#include <LittleFS.h>
#endif
#include <Preferences.h>
#include <SD_MMC.h>
#include <SD.h>
#include <SPI.h>
#include <cstring>

static Preferences prefs;
static bool nvsReady = false;
static bool sdReady = false;
static fs::FS* sdFs = nullptr;
static bool fileReady = false;
static fs::FS* fileFs = nullptr;

struct SdIoContext {
    fs::FS* fs;
    File file;
};

static bool sdIoExists(void* ctx, const char* path) {
    SdIoContext* io = static_cast<SdIoContext*>(ctx);
    return io->fs->exists(path);
}

static bool sdIoRemove(void* ctx, const char* path) {
    SdIoContext* io = static_cast<SdIoContext*>(ctx);
    return io->fs->remove(path);
}

static bool sdIoOpenWrite(void* ctx, const char* path, bool append) {
    SdIoContext* io = static_cast<SdIoContext*>(ctx);
    io->file = io->fs->open(path, append ? FILE_APPEND : FILE_WRITE);
    return static_cast<bool>(io->file);
}

static bool sdIoWriteText(void* ctx, const char* content) {
    SdIoContext* io = static_cast<SdIoContext*>(ctx);
    size_t written = io->file.print(content);
    return written > 0 || std::strlen(content) == 0;
}

static void sdIoClose(void* ctx) {
    SdIoContext* io = static_cast<SdIoContext*>(ctx);
    if (io->file) {
        io->file.close();
    }
}

#ifndef FILE_APPEND
#define FILE_APPEND FILE_WRITE
#endif

// ===== NVS =====

void storageInit() {
    Serial.println("[Storage] Initializing NVS...");
    nvsReady = prefs.begin(NVS_NAMESPACE, false);
    if (nvsReady) {
        Serial.println("[Storage] NVS ready.");
    } else {
        Serial.println("[Storage] NVS init failed.");
    }

    sdReady = false;
    sdFs = nullptr;
    fileReady = false;
    fileFs = nullptr;

    const BoardProfile& board = boardGetProfile();
    if (!board.sd.supported) {
        Serial.println("[Storage] SD is not supported on this board.");
#if FEATURE_LITTLEFS
        Serial.println("[Storage] Initializing LittleFS fallback...");
        if (LittleFS.begin(true)) {
            fileReady = true;
            fileFs = &LittleFS;
            Serial.printf(
                "[Storage] LittleFS ready. Used: %u / %u bytes\n",
                static_cast<unsigned>(LittleFS.usedBytes()),
                static_cast<unsigned>(LittleFS.totalBytes())
            );
        } else {
            Serial.println("[Storage] LittleFS mount failed.");
        }
#else
        Serial.println("[Storage] LittleFS fallback is disabled by build profile.");
#endif
        return;
    }

    if (board.sd.useSdMmc1Bit) {
        Serial.println("[Storage] Initializing SD card (SDMMC 1-bit)...");
        if (!SD_MMC.setPins(board.sd.sdmmcClkPin, board.sd.sdmmcCmdPin, board.sd.sdmmcD0Pin)) {
            Serial.println("[Storage] SD_MMC pin setup failed.");
        } else if (SD_MMC.begin("/sdcard", true, false, SDMMC_FREQ_DEFAULT)) {
            sdReady = true;
            sdFs = &SD_MMC;
            fileReady = true;
            fileFs = sdFs;
            Serial.printf("[Storage] SD card ready. Size: %llu MB\n", SD_MMC.cardSize() / (1024ULL * 1024ULL));
        } else {
            Serial.println("[Storage] SD card mount failed.");
            Serial.println("[Storage] Hint: format card as FAT32 (MBR), not exFAT.");
        }
        return;
    }

#if defined(SD_CS)
    Serial.println("[Storage] Initializing SD card (SPI)...");
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    if (SD.begin(SD_CS)) {
        sdReady = true;
        sdFs = &SD;
        fileReady = true;
        fileFs = sdFs;
        Serial.println("[Storage] SD card ready.");
    } else {
        Serial.println("[Storage] SD card not found, skipping.");
        Serial.println("[Storage] Hint: format card as FAT32 (MBR), not exFAT.");
    }
#else
    Serial.println("[Storage] SD backend is not configured for this board profile.");
#endif
}

bool storageNvsAvailable() {
    return nvsReady;
}

void storageSetString(const char* key, const char* value) {
    if (!nvsReady) {
        Serial.println("[Storage] NVS not ready, skip putString.");
        return;
    }
    prefs.putString(key, value);
}

String storageGetString(const char* key, const char* defaultValue) {
    if (!nvsReady) return String(defaultValue);
    return prefs.getString(key, defaultValue);
}

void storageSetInt(const char* key, int value) {
    if (!nvsReady) {
        Serial.println("[Storage] NVS not ready, skip putInt.");
        return;
    }
    prefs.putInt(key, value);
}

int storageGetInt(const char* key, int defaultValue) {
    if (!nvsReady) return defaultValue;
    return prefs.getInt(key, defaultValue);
}

// ===== SD =====

bool sdSupported() {
    return boardGetProfile().sd.supported;
}

bool sdAvailable() {
    return sdReady;
}

String sdReadFile(const char* path) {
    if (!fileReady || fileFs == nullptr) return "";
    File file = fileFs->open(path);
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
    if (!fileReady || fileFs == nullptr) return false;
    SdIoContext ctx = { fileFs, File() };
    StorageIo io = { sdIoExists, sdIoRemove, sdIoOpenWrite, sdIoWriteText, sdIoClose, &ctx };
    StorageIoResult result = storageWriteText(&io, path, content, StorageWriteMode::Overwrite);
    if (result == StorageIoResult::Ok) {
        return true;
    }
    Serial.printf("[Storage] Write failed (%d): %s\n", static_cast<int>(result), path);
    return false;
}

bool sdAppendFile(const char* path, const char* content) {
    if (!fileReady || fileFs == nullptr) return false;
    SdIoContext ctx = { fileFs, File() };
    StorageIo io = { sdIoExists, sdIoRemove, sdIoOpenWrite, sdIoWriteText, sdIoClose, &ctx };
    StorageIoResult result = storageWriteText(&io, path, content, StorageWriteMode::Append);
    if (result == StorageIoResult::Ok) {
        return true;
    }
    Serial.printf("[Storage] Append failed (%d): %s\n", static_cast<int>(result), path);
    return false;
}
