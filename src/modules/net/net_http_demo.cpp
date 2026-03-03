#include "net_http_demo.h"

#include <Arduino.h>

#include "../../config.h"

#if FEATURE_NET_HTTP
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#endif

#if FEATURE_NET_HTTP

namespace {

bool initialized = false;
bool lastRequestOk = false;
unsigned long lastAttemptMs = 0;
unsigned long lastSuccessMs = 0;
String lastValue = "-";
String uiText = "NET:-";
bool fetchInProgress = false;
WiFiClient plainClient;
WiFiClientSecure secureClient;
SemaphoreHandle_t stateMutex = nullptr;
TaskHandle_t fetchTaskHandle = nullptr;

constexpr uint32_t kFetchTaskStackBytes = 6144;
constexpr UBaseType_t kFetchTaskPriority = 1;

String trimForUi(const String& value) {
    String trimmed = value;
    trimmed.trim();
    if (trimmed.length() == 0) {
        return "-";
    }

    if (trimmed.length() <= NET_DEMO_UI_MAX_VALUE_LEN) {
        return trimmed;
    }

    return trimmed.substring(0, NET_DEMO_UI_MAX_VALUE_LEN);
}

bool extractJsonStringField(const String& json, const char* key, String& outValue) {
    const String keyToken = String("\"") + key + "\"";
    const int keyPos = json.indexOf(keyToken);
    if (keyPos < 0) {
        return false;
    }

    const int colonPos = json.indexOf(':', keyPos + keyToken.length());
    if (colonPos < 0) {
        return false;
    }

    const int quotePos = json.indexOf('"', colonPos + 1);
    if (quotePos < 0) {
        return false;
    }

    String parsed = "";
    parsed.reserve(NET_DEMO_UI_MAX_VALUE_LEN + 8);

    for (int i = quotePos + 1; i < json.length(); i++) {
        const char c = json.charAt(i);

        if (c == '"') {
            outValue = parsed;
            return true;
        }

        if (c == '\\') {
            if (i + 1 >= json.length()) {
                return false;
            }
            const char escaped = json.charAt(i + 1);
            parsed += escaped;
            i++;
            continue;
        }

        parsed += c;
    }

    return false;
}

void updateUiText() {
    if (lastAttemptMs == 0) {
        uiText = "NET:-";
        return;
    }

    if (lastRequestOk) {
        uiText = String("NET:") + trimForUi(lastValue);
        return;
    }

    uiText = "NET:ERR";
}

bool lockState(TickType_t waitTicks = portMAX_DELAY) {
    if (stateMutex == nullptr) {
        return false;
    }
    return xSemaphoreTake(stateMutex, waitTicks) == pdTRUE;
}

void unlockState() {
    xSemaphoreGive(stateMutex);
}

void applyFetchFailure() {
    if (lockState()) {
        lastRequestOk = false;
        updateUiText();
        fetchInProgress = false;
        fetchTaskHandle = nullptr;
        unlockState();
    }
}

void applyFetchSuccess(const String& extractedValue) {
    if (lockState()) {
        lastRequestOk = true;
        lastValue = extractedValue;
        lastSuccessMs = millis();
        updateUiText();
        fetchInProgress = false;
        fetchTaskHandle = nullptr;
        unlockState();
    }
}

void performFetch() {
    HTTPClient http;
    http.setConnectTimeout(NET_DEMO_HTTP_TIMEOUT_MS);
    http.setTimeout(NET_DEMO_HTTP_TIMEOUT_MS);
    const String url = NET_DEMO_URL;

    bool beginOk = false;
    if (url.startsWith("https://")) {
        secureClient.setInsecure();
        beginOk = http.begin(secureClient, url);
    } else {
        beginOk = http.begin(plainClient, url);
    }
    if (!beginOk) {
        applyFetchFailure();
        Serial.println("[NET] HTTP begin failed");
        return;
    }

    http.addHeader("Accept", "application/json");
    http.setUserAgent(APP_NAME "/" APP_VERSION);

    const int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        applyFetchFailure();
        Serial.printf("[NET] HTTP error: %d\n", httpCode);
        http.end();
        return;
    }

    const String payload = http.getString();
    http.end();

    String extracted = "";
    if (!extractJsonStringField(payload, NET_DEMO_JSON_KEY, extracted)) {
        applyFetchFailure();
        Serial.printf("[NET] JSON parse failed for key '%s'\n", NET_DEMO_JSON_KEY);
        return;
    }

    applyFetchSuccess(extracted);
    Serial.printf("[NET] %s=%s\n", NET_DEMO_JSON_KEY, extracted.c_str());
}

void fetchTaskEntry(void* /*param*/) {
    performFetch();
    vTaskDelete(nullptr);
}

} // namespace

void netDemoInit() {
    if (initialized) {
        return;
    }

    if (stateMutex == nullptr) {
        stateMutex = xSemaphoreCreateMutex();
    }
    if (stateMutex == nullptr) {
        Serial.println("[NET] init failed: cannot create mutex");
        return;
    }

    if (lockState()) {
        fetchInProgress = false;
        fetchTaskHandle = nullptr;
        lastRequestOk = false;
        lastAttemptMs = 0;
        lastSuccessMs = 0;
        lastValue = "-";
        uiText = "NET:-";
        unlockState();
    }

    initialized = true;
}

void netDemoLoop(bool internetAvailable) {
    if (!initialized) {
        netDemoInit();
    }

    if (!internetAvailable) {
        return;
    }

    bool shouldStartFetch = false;
    const unsigned long nowMs = millis();

    if (!lockState()) {
        return;
    }

    if (!fetchInProgress
        && (lastAttemptMs == 0 || nowMs - lastAttemptMs >= NET_DEMO_REQUEST_INTERVAL_MS)) {
        fetchInProgress = true;
        lastAttemptMs = nowMs;
        updateUiText();
        shouldStartFetch = true;
    }
    unlockState();

    if (!shouldStartFetch) {
        return;
    }

    const BaseType_t created = xTaskCreatePinnedToCore(
        fetchTaskEntry,
        "netFetch",
        kFetchTaskStackBytes,
        nullptr,
        kFetchTaskPriority,
        &fetchTaskHandle,
        tskNO_AFFINITY
    );
    if (created != pdPASS) {
        applyFetchFailure();
        Serial.println("[NET] fetch task create failed");
    }
}

bool netDemoIsEnabled() {
    return true;
}

bool netDemoLastRequestOk() {
    bool value = false;
    if (lockState()) {
        value = lastRequestOk;
        unlockState();
    }
    return value;
}

String netDemoGetValue() {
    String value = "-";
    if (lockState()) {
        value = lastRequestOk ? lastValue : String("-");
        unlockState();
    }
    return value;
}

String netDemoGetUiText() {
    String value = "NET:-";
    if (lockState()) {
        value = uiText;
        unlockState();
    }
    return value;
}

unsigned long netDemoLastAttemptMs() {
    unsigned long value = 0;
    if (lockState()) {
        value = lastAttemptMs;
        unlockState();
    }
    return value;
}

unsigned long netDemoLastSuccessMs() {
    unsigned long value = 0;
    if (lockState()) {
        value = lastSuccessMs;
        unlockState();
    }
    return value;
}

#else

void netDemoInit() {}

void netDemoLoop(bool internetAvailable) {
    (void) internetAvailable;
}

bool netDemoIsEnabled() {
    return false;
}

bool netDemoLastRequestOk() {
    return false;
}

String netDemoGetValue() {
    return String("-");
}

String netDemoGetUiText() {
    return String("NET:OFF");
}

unsigned long netDemoLastAttemptMs() {
    return 0;
}

unsigned long netDemoLastSuccessMs() {
    return 0;
}

#endif
