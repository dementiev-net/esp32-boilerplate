#pragma once

#include <Arduino.h>

/**
 * @brief Инициализирует Wi-Fi в режиме AUTO/AP/STA.
 * @details
 * - Сначала пытается прочитать `/wifi.conf` из активного файлового backend.
 * - T-Display-S3: файл читается с SD.
 * - T-QT Pro: при `FEATURE_LITTLEFS=1` файл читается из LittleFS.
 * - При отсутствии валидного конфига использует AUTO (WiFiManager).
 * @note Вызывать один раз из setup(). Не ISR-safe.
 */
void wifiInit();

/**
 * @brief Периодическая обработка Wi-Fi состояния.
 * @details Вызывать регулярно из loop() для портала и реконнекта.
 */
void wifiLoop();

/**
 * @brief Возвращает состояние подключения.
 * @return true если STA подключен или активен AP режим.
 */
bool wifiIsConnected();

/**
 * @brief Возвращает IP текущего режима.
 * @return STA IP в STA/AUTO, AP IP в AP режиме.
 */
String wifiGetIP();

/**
 * @brief Возвращает SSID текущего режима.
 * @return SSID роутера в STA/AUTO или SSID точки доступа в AP.
 */
String wifiGetSSID();

/**
 * @brief Возвращает короткую метку режима.
 * @return "AUTO", "AP" или "STA".
 */
const char* wifiGetModeLabel();

/**
 * @brief Проверяет, активен ли режим STA.
 */
bool wifiIsStaMode();

/**
 * @brief Проверяет, активен ли режим AP.
 */
bool wifiIsApMode();
