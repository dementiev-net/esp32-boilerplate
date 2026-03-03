#include "net_time.h"

#include "../../config.h"

#include <time.h>

static bool ntpStarted = false;
static bool ntpSynced = false;
static unsigned long lastAttemptMs = 0;
static unsigned long lastProbeMs = 0;

static void startNtpSync(unsigned long nowMs) {
    configTzTime(NTP_TIMEZONE, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);
    ntpStarted = true;
    lastAttemptMs = nowMs;
    Serial.printf(
        "[Time] NTP sync requested (tz=%s, server=%s)\n",
        NTP_TIMEZONE,
        NTP_SERVER_1
    );
}

void netTimeInit() {
    ntpStarted = false;
    ntpSynced = false;
    lastAttemptMs = 0;
    lastProbeMs = 0;
}

void netTimeLoop(bool internetAvailable) {
    if (!internetAvailable) {
        return;
    }

    const unsigned long nowMs = millis();
    if (!ntpStarted || (!ntpSynced && (nowMs - lastAttemptMs >= NTP_RETRY_MS))) {
        startNtpSync(nowMs);
    }

    if (nowMs - lastProbeMs < 250) {
        return;
    }
    lastProbeMs = nowMs;

    const time_t now = time(nullptr);
    if (now >= static_cast<time_t>(NTP_VALID_UNIX_TS)) {
        if (!ntpSynced) {
            ntpSynced = true;
            Serial.printf("[Time] NTP synced. Epoch=%lu\n", static_cast<unsigned long>(now));
        }
    }
}

bool netTimeIsSynced() {
    return ntpSynced;
}

String netTimeGetLocalTimeText() {
    if (!ntpSynced) {
        return "--:--:--";
    }

    const time_t now = time(nullptr);
    struct tm localTm;
    if (!localtime_r(&now, &localTm)) {
        return "--:--:--";
    }

    char buffer[16];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &localTm);
    return String(buffer);
}
