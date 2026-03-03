#include "net_http_demo.h"

#include <Arduino.h>

#include "../../config.h"

#if FEATURE_NET_HTTP
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#endif

#if FEATURE_NET_HTTP

namespace {

bool initialized = false;
bool lastRequestOk = false;
unsigned long lastAttemptMs = 0;
unsigned long lastSuccessMs = 0;
String lastValue = "-";
String uiText = "NET:-";
WiFiClient plainClient;
WiFiClientSecure secureClient;

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

void performFetch() {
    lastAttemptMs = millis();

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
        lastRequestOk = false;
        updateUiText();
        Serial.println("[NET] HTTP begin failed");
        return;
    }

    http.addHeader("Accept", "application/json");
    http.setUserAgent(APP_NAME "/" APP_VERSION);

    const int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        lastRequestOk = false;
        updateUiText();
        Serial.printf("[NET] HTTP error: %d\n", httpCode);
        http.end();
        return;
    }

    const String payload = http.getString();
    http.end();

    String extracted = "";
    if (!extractJsonStringField(payload, NET_DEMO_JSON_KEY, extracted)) {
        lastRequestOk = false;
        updateUiText();
        Serial.printf("[NET] JSON parse failed for key '%s'\n", NET_DEMO_JSON_KEY);
        return;
    }

    lastValue = extracted;
    lastRequestOk = true;
    lastSuccessMs = millis();
    updateUiText();
    Serial.printf("[NET] %s=%s\n", NET_DEMO_JSON_KEY, lastValue.c_str());
}

} // namespace

void netDemoInit() {
    initialized = true;
    lastRequestOk = false;
    lastAttemptMs = 0;
    lastSuccessMs = 0;
    lastValue = "-";
    uiText = "NET:-";
}

void netDemoLoop(bool internetAvailable) {
    if (!initialized) {
        netDemoInit();
    }

    if (!internetAvailable) {
        return;
    }

    const unsigned long nowMs = millis();
    if (lastAttemptMs != 0 && nowMs - lastAttemptMs < NET_DEMO_REQUEST_INTERVAL_MS) {
        return;
    }

    performFetch();
}

bool netDemoIsEnabled() {
    return true;
}

bool netDemoLastRequestOk() {
    return lastRequestOk;
}

String netDemoGetValue() {
    return lastRequestOk ? lastValue : String("-");
}

String netDemoGetUiText() {
    return uiText;
}

unsigned long netDemoLastAttemptMs() {
    return lastAttemptMs;
}

unsigned long netDemoLastSuccessMs() {
    return lastSuccessMs;
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
