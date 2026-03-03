#pragma once

#include <string>

/**
 * @brief Режим работы Wi-Fi из конфигурации.
 */
enum class WifiConfigMode {
    AUTO,
    AP,
    STA
};

/**
 * @brief Значения по умолчанию для AP-параметров.
 */
struct WifiConfigDefaults {
    std::string apSsid;
    std::string apPassword;
};

/**
 * @brief Результат парсинга wifi.conf.
 */
struct WifiConfigData {
    WifiConfigMode mode = WifiConfigMode::AUTO;
    std::string staSsid;
    std::string staPassword;
    std::string apSsid;
    std::string apPassword;
};

/**
 * @brief Парсит текст wifi.conf.
 * @details Поддерживаются ключи mode, sta_ssid, sta_password, ap_ssid, ap_password.
 * @param rawConfig Текст конфигурации.
 * @param defaults Значения по умолчанию для AP.
 * @param out Структура результата.
 * @return true если найден валидный mode и конфиг принят.
 */
bool parseWifiConfigText(
    const std::string& rawConfig,
    const WifiConfigDefaults& defaults,
    WifiConfigData& out
);
