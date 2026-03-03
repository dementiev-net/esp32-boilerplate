#include "app_runtime.h"

#include "../../config.h"
#include "../power/battery.h"

void appRuntimeInit() {
    appDispatcherInit();
}

bool appRuntimePostEvent(AppEventType type) {
    return appDispatcherPost(type);
}

void appRuntimePublishStateChanges(
    const RuntimeSnapshot& previous,
    const RuntimeSnapshot& current
) {
    bool needUiRefresh = false;

    if (previous.topPressed != current.topPressed || previous.bottomPressed != current.bottomPressed) {
        appRuntimePostEvent(AppEventType::ButtonsStateChanged);
        needUiRefresh = true;
    }

    if (previous.topHoldActive != current.topHoldActive || previous.bottomHoldActive != current.bottomHoldActive) {
        needUiRefresh = true;
    }

    if (previous.wifiConnected != current.wifiConnected) {
        appRuntimePostEvent(AppEventType::WifiConnectionChanged);
        needUiRefresh = true;
    }

    if (previous.modeLabel != current.modeLabel || previous.ip != current.ip || previous.ssid != current.ssid) {
        appRuntimePostEvent(AppEventType::WifiIdentityChanged);
        needUiRefresh = true;
    }

    if (previous.sdSupported != current.sdSupported || previous.sdAvailable != current.sdAvailable) {
        appRuntimePostEvent(AppEventType::SdStateChanged);
        needUiRefresh = true;
    }

    if (previous.timeSynced != current.timeSynced || previous.localTime != current.localTime) {
        appRuntimePostEvent(AppEventType::TimeUpdated);
        needUiRefresh = true;
    }

    if (previous.batterySupported != current.batterySupported
        || previous.batteryMillivolts != current.batteryMillivolts) {
        appRuntimePostEvent(AppEventType::BatteryUpdated);
        needUiRefresh = true;
    }

    if (needUiRefresh) {
        appRuntimePostEvent(AppEventType::UiRefreshRequested);
    }
}

AppRuntimeDrainResult appRuntimeDrainEvents(
    RuntimeStateTracker& tracker,
    const RuntimeSnapshot& runtimeState,
    unsigned long nowMs
) {
    AppRuntimeDrainResult result;
    AppEvent event;

    while (appDispatcherNext(event)) {
        switch (event.type) {
            case AppEventType::ButtonTopClick:
                Serial.println("[Button] Top click");
                result.needUiRefresh = true;
                break;
            case AppEventType::ButtonTopLongPress:
                Serial.println("[Button] Top long press");
                result.needUiRefresh = true;
                break;
            case AppEventType::ButtonTopHold:
                runtimeStateActivateTopHold(tracker, nowMs, BUTTON_HOLD_INDICATOR_MS);
                result.needUiRefresh = true;
                break;
            case AppEventType::ButtonBottomClick:
                Serial.println("[Button] Bottom click");
                result.needUiRefresh = true;
                break;
            case AppEventType::ButtonBottomLongPress:
                Serial.println("[Button] Bottom long press");
                result.needUiRefresh = true;
                break;
            case AppEventType::ButtonBottomHold:
                runtimeStateActivateBottomHold(tracker, nowMs, BUTTON_HOLD_INDICATOR_MS);
                result.needUiRefresh = true;
                break;
            case AppEventType::ButtonsStateChanged:
                Serial.printf(
                    "[Runtime] Buttons state T:%d B:%d\n",
                    runtimeState.topPressed ? 1 : 0,
                    runtimeState.bottomPressed ? 1 : 0
                );
                break;
            case AppEventType::WifiConnectionChanged:
                Serial.printf(
                    "[Runtime] WiFi %s\n",
                    runtimeState.wifiConnected ? "connected" : "disconnected"
                );
                break;
            case AppEventType::WifiIdentityChanged:
                Serial.printf(
                    "[Runtime] WiFi mode=%s ssid=%s ip=%s\n",
                    runtimeState.modeLabel.c_str(),
                    runtimeState.ssid.c_str(),
                    runtimeState.ip.c_str()
                );
                break;
            case AppEventType::SdStateChanged:
                Serial.printf(
                    "[Runtime] SD supported=%d ready=%d\n",
                    runtimeState.sdSupported ? 1 : 0,
                    runtimeState.sdAvailable ? 1 : 0
                );
                break;
            case AppEventType::TimeUpdated:
                break;
            case AppEventType::BatteryUpdated:
                if (runtimeState.batterySupported && runtimeState.batteryMillivolts >= 0) {
                    const int percent = batteryMillivoltsToPercent(runtimeState.batteryMillivolts);
                    Serial.printf("[Runtime] VBAT=%d%% (%dmV)\n", percent, runtimeState.batteryMillivolts);
                } else {
                    Serial.println("[Runtime] VBAT unavailable");
                }
                break;
            case AppEventType::UiRefreshRequested:
                result.needUiRefresh = true;
                break;
            case AppEventType::None:
            default:
                break;
        }
    }

    return result;
}

size_t appRuntimeDroppedEvents() {
    return appDispatcherDroppedCount();
}
