#include "runtime_state.h"

#include "../../config.h"
#include "../buttons/buttons.h"
#include "../power/battery.h"
#include "../storage/storage.h"
#include "../time/net_time.h"
#include "../wifi/wifi.h"

static bool isFutureOrNow(unsigned long nowMs, unsigned long deadlineMs) {
    return static_cast<long>(deadlineMs - nowMs) >= 0;
}

void runtimeStateInit(RuntimeStateTracker& tracker) {
    tracker.topHoldUntilMs = 0;
    tracker.bottomHoldUntilMs = 0;
    tracker.cachedUiTime = "--:--:--";
    tracker.lastTimeUiSampleMs = 0;
    tracker.cachedBatteryMillivolts = -1;
    tracker.lastBatterySampleMs = 0;
    tracker.batterySampleInitialized = false;
}

void runtimeStateActivateTopHold(RuntimeStateTracker& tracker, unsigned long nowMs, unsigned long indicatorMs) {
    tracker.topHoldUntilMs = nowMs + indicatorMs;
}

void runtimeStateActivateBottomHold(RuntimeStateTracker& tracker, unsigned long nowMs, unsigned long indicatorMs) {
    tracker.bottomHoldUntilMs = nowMs + indicatorMs;
}

RuntimeSnapshot runtimeStateRead(RuntimeStateTracker& tracker, unsigned long nowMs) {
    RuntimeSnapshot state;

    state.topPressed = buttonsIsTopPressed();
    state.bottomPressed = buttonsIsBottomPressed();
    state.topHoldActive = isFutureOrNow(nowMs, tracker.topHoldUntilMs);
    state.bottomHoldActive = isFutureOrNow(nowMs, tracker.bottomHoldUntilMs);
    state.wifiConnected = wifiIsConnected();
    state.sdSupported = sdSupported();
    state.sdAvailable = sdAvailable();
    state.modeLabel = wifiGetModeLabel();
    state.ip = state.wifiConnected ? wifiGetIP() : "-";
    state.ssid = wifiGetSSID();
    state.timeSynced = netTimeIsSynced();
    state.batterySupported = batteryIsSupported();

    if (!state.timeSynced) {
        tracker.cachedUiTime = "--:--:--";
    } else if (nowMs - tracker.lastTimeUiSampleMs >= 1000 || tracker.cachedUiTime == "--:--:--") {
        tracker.cachedUiTime = netTimeGetLocalTimeText();
        tracker.lastTimeUiSampleMs = nowMs;
    }
    state.localTime = tracker.cachedUiTime;

    if (!state.batterySupported) {
        tracker.cachedBatteryMillivolts = -1;
        tracker.batterySampleInitialized = false;
    } else if (!tracker.batterySampleInitialized
               || nowMs - tracker.lastBatterySampleMs >= BATTERY_SAMPLE_INTERVAL_MS) {
        tracker.cachedBatteryMillivolts = batteryReadMillivolts();
        tracker.lastBatterySampleMs = nowMs;
        tracker.batterySampleInitialized = true;
    }
    state.batteryMillivolts = tracker.cachedBatteryMillivolts;
    state.batteryPercent = batteryMillivoltsToPercent(state.batteryMillivolts);

    return state;
}

bool runtimeStateEquals(const RuntimeSnapshot& a, const RuntimeSnapshot& b) {
    return a.topPressed == b.topPressed
        && a.bottomPressed == b.bottomPressed
        && a.topHoldActive == b.topHoldActive
        && a.bottomHoldActive == b.bottomHoldActive
        && a.wifiConnected == b.wifiConnected
        && a.sdSupported == b.sdSupported
        && a.sdAvailable == b.sdAvailable
        && a.modeLabel == b.modeLabel
        && a.ip == b.ip
        && a.ssid == b.ssid
        && a.timeSynced == b.timeSynced
        && a.localTime == b.localTime
        && a.batterySupported == b.batterySupported
        && a.batteryMillivolts == b.batteryMillivolts
        && a.batteryPercent == b.batteryPercent;
}
