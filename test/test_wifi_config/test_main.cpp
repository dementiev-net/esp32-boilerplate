#include <unity.h>

#include "modules/wifi/wifi_config.h"

void test_parse_fails_without_mode() {
    WifiConfigDefaults defaults = { "DefaultAP", "12345678" };
    WifiConfigData out;

    bool ok = parseWifiConfigText("sta_ssid=MyWiFi\n", defaults, out);

    TEST_ASSERT_FALSE(ok);
}

void test_parse_sta_mode_with_credentials() {
    WifiConfigDefaults defaults = { "DefaultAP", "12345678" };
    WifiConfigData out;

    bool ok = parseWifiConfigText(
        "mode=sta\n"
        "sta_ssid=HomeWiFi\n"
        "sta_password=secret\n",
        defaults,
        out
    );

    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(WifiConfigMode::STA), static_cast<int>(out.mode));
    TEST_ASSERT_EQUAL_STRING("HomeWiFi", out.staSsid.c_str());
    TEST_ASSERT_EQUAL_STRING("secret", out.staPassword.c_str());
    TEST_ASSERT_EQUAL_STRING("DefaultAP", out.apSsid.c_str());
    TEST_ASSERT_EQUAL_STRING("12345678", out.apPassword.c_str());
}

void test_parse_ap_mode_uses_defaults_for_short_password() {
    WifiConfigDefaults defaults = { "DefaultAP", "12345678" };
    WifiConfigData out;

    bool ok = parseWifiConfigText(
        "mode=ap\n"
        "ap_ssid=CustomAP\n"
        "ap_password=1234\n",
        defaults,
        out
    );

    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(WifiConfigMode::AP), static_cast<int>(out.mode));
    TEST_ASSERT_EQUAL_STRING("CustomAP", out.apSsid.c_str());
    TEST_ASSERT_EQUAL_STRING("12345678", out.apPassword.c_str());
}

void test_parse_is_case_insensitive_and_handles_quotes() {
    WifiConfigDefaults defaults = { "DefaultAP", "12345678" };
    WifiConfigData out;

    bool ok = parseWifiConfigText(
        "  MODE = \"StA\" \n"
        " sta_ssid = \"Office WiFi\" \n"
        " sta_password = \"pa55\" \n"
        "; comment\n"
        "# comment\n"
        "unknown_key=value\n",
        defaults,
        out
    );

    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(WifiConfigMode::STA), static_cast<int>(out.mode));
    TEST_ASSERT_EQUAL_STRING("Office WiFi", out.staSsid.c_str());
    TEST_ASSERT_EQUAL_STRING("pa55", out.staPassword.c_str());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_parse_fails_without_mode);
    RUN_TEST(test_parse_sta_mode_with_credentials);
    RUN_TEST(test_parse_ap_mode_uses_defaults_for_short_password);
    RUN_TEST(test_parse_is_case_insensitive_and_handles_quotes);
    return UNITY_END();
}
