#pragma once

// ===== VERSION =====
/// @brief Версия приложения для вывода на экран/в логи.
#define APP_VERSION "1.2.13"
/// @brief Короткое имя приложения для UI/логов.
#define APP_NAME    "Boilerplate"

// ===== FEATURES (BUILD PROFILE) =====
/// @brief Включение OTA модуля: 1 = включен, 0 = исключен из runtime.
#ifndef FEATURE_OTA
  #define FEATURE_OTA 1
#endif
/// @brief Включение BLE модуля: 1 = включен, 0 = исключен из runtime.
#ifndef FEATURE_BLE
  #define FEATURE_BLE 1
#endif
/// @brief Включение fallback-файловой системы LittleFS: 1 = включен, 0 = выключен.
#ifndef FEATURE_LITTLEFS
  #define FEATURE_LITTLEFS 0
#endif
/// @brief Включение USB CDC shell: 1 = включен, 0 = выключен.
#ifndef FEATURE_USB_SHELL
  #define FEATURE_USB_SHELL 1
#endif
/// @brief Включение demo HTTP JSON клиента: 1 = включен, 0 = исключен из runtime.
#ifndef FEATURE_NET_HTTP
  #define FEATURE_NET_HTTP 1
#endif
/// @brief Включение watchdog/diagnostics модуля: 1 = включен, 0 = исключен из runtime.
#ifndef FEATURE_WATCHDOG
  #define FEATURE_WATCHDOG 1
#endif

// ===== SERIAL =====
/// @brief Скорость UART (baud) для Serial Monitor.
#define SERIAL_BAUD_RATE 115200

// ===== WIFI =====
/// @brief SSID точки доступа по умолчанию (режим AP/AUTO).
#define WIFI_AP_NAME     "ESP32-Boilerplate"
/// @brief Пароль точки доступа по умолчанию (минимум 8 символов).
#define WIFI_AP_PASSWORD "12345678"
/// @brief Таймаут подключения в режиме STA, миллисекунды.
#define WIFI_TIMEOUT_MS  10000
/// @brief Путь до Wi-Fi конфига на SD-карте.
#define WIFI_CONFIG_FILE "/wifi.conf"

// ===== TIME (NTP) =====
/// @brief POSIX timezone для локального отображения NTP-времени.
#define NTP_TIMEZONE "UTC0"
/// @brief Основной NTP сервер.
#define NTP_SERVER_1 "pool.ntp.org"
/// @brief Резервный NTP сервер №2.
#define NTP_SERVER_2 "time.google.com"
/// @brief Резервный NTP сервер №3.
#define NTP_SERVER_3 "time.nist.gov"
/// @brief Интервал повторной попытки NTP-синхронизации, миллисекунды.
#define NTP_RETRY_MS 15000
/// @brief Минимальный UNIX timestamp для признания времени валидным.
#define NTP_VALID_UNIX_TS 1700000000UL

// ===== NET (HTTP JSON DEMO) =====
/// @brief URL demo endpoint для HTTP JSON запроса.
#define NET_DEMO_URL "https://api.coinbase.com/v2/prices/BTC-USD/spot"
/// @brief JSON ключ, который извлекается из ответа endpoint.
#define NET_DEMO_JSON_KEY "amount"
/// @brief Интервал между HTTP запросами, миллисекунды.
#define NET_DEMO_REQUEST_INTERVAL_MS 60000
/// @brief Таймаут HTTP подключения/чтения, миллисекунды.
#define NET_DEMO_HTTP_TIMEOUT_MS 2500
/// @brief Максимальная длина значения в UI (после префикса `NET:`), символы.
#define NET_DEMO_UI_MAX_VALUE_LEN 18

// ===== DISPLAY =====
/// @brief Минимальное время показа boot-заставки, миллисекунды.
#define BOOT_SPLASH_MIN_MS 1800

// ===== BATTERY =====
/// @brief Период выборки напряжения батареи в runtime, миллисекунды.
#define BATTERY_SAMPLE_INTERVAL_MS 3000
/// @brief Минимально валидное напряжение батареи, милливольты.
#define BATTERY_MIN_VALID_MV 2500
/// @brief Максимально валидное напряжение батареи, милливольты.
#define BATTERY_MAX_VALID_MV 4300
/// @brief Коэффициент делителя для пересчета ADC -> VBAT (числитель).
#define BATTERY_DIVIDER_NUM 2
/// @brief Коэффициент делителя для пересчета ADC -> VBAT (знаменатель).
#define BATTERY_DIVIDER_DEN 1
/// @brief Калибровочный множитель VBAT после делителя (числитель, промилле = 1000).
#define BATTERY_CALIBRATION_NUM 1035
/// @brief Калибровочный множитель VBAT после делителя (знаменатель, промилле = 1000).
#define BATTERY_CALIBRATION_DEN 1000
/// @brief Напряжение пустой Li-ion батареи для расчета процента, милливольты.
#define BATTERY_PERCENT_EMPTY_MV 3300
/// @brief Напряжение полной Li-ion батареи для расчета процента, милливольты.
#define BATTERY_PERCENT_FULL_MV 4200

// ===== RELIABILITY (WATCHDOG / RESET DIAG) =====
/// @brief Таймаут watchdog для loop-задачи, секунды.
#define WATCHDOG_TIMEOUT_SEC 10
/// @brief Период feed watchdog из `loop()`, миллисекунды.
#define WATCHDOG_FEED_INTERVAL_MS 1000

#ifdef BOARD_TDISPLAY_S3
  /// @brief Ширина экрана в пикселях (landscape).
  #define SCREEN_WIDTH    320
  /// @brief Высота экрана в пикселях (landscape).
  #define SCREEN_HEIGHT   170
  /// @brief Поворот дисплея для TFT_eSPI.
  #define SCREEN_ROTATION 1
#endif

#ifdef BOARD_TQT_PRO
  /// @brief Ширина экрана в пикселях.
  #define SCREEN_WIDTH    128
  /// @brief Высота экрана в пикселях.
  #define SCREEN_HEIGHT   128
  /// @brief Поворот дисплея для TFT_eSPI.
  #define SCREEN_ROTATION 2
#endif

// ===== BUTTONS =====
/// @brief Время антидребезга кнопок, миллисекунды.
#define BUTTON_DEBOUNCE_MS 50
/// @brief Порог долгого нажатия, миллисекунды.
#define BUTTON_LONG_PRESS_MS 800
/// @brief Интервал событий удерживания после long-press, миллисекунды.
#define BUTTON_HOLD_REPEAT_MS 250
/// @brief Время отображения индикатора hold на экране, миллисекунды.
#define BUTTON_HOLD_INDICATOR_MS 350

#ifdef BOARD_TDISPLAY_S3
  /// @brief GPIO верхней кнопки.
  #define BTN_TOP    14
  /// @brief GPIO нижней кнопки.
  #define BTN_BOTTOM  0
#endif

#ifdef BOARD_TQT_PRO
  /// @brief GPIO верхней кнопки.
  #define BTN_TOP     0
  /// @brief GPIO нижней кнопки.
  #define BTN_BOTTOM 47
#endif

// ===== SD CARD =====
#ifdef BOARD_TDISPLAY_S3
  /// @brief Использовать SDMMC 1-bit режим для LILYGO TF board.
  #define SD_USE_SDMMC_1BIT 1
  /// @brief SDMMC CMD GPIO.
  #define SDMMC_CMD 13
  /// @brief SDMMC CLK GPIO.
  #define SDMMC_CLK 11
  /// @brief SDMMC D0 GPIO.
  #define SDMMC_D0  12
#endif

// ===== STORAGE =====
/// @brief Namespace для Preferences (NVS).
#define NVS_NAMESPACE "boilerplate"

// ===== OTA =====
/// @brief Порт OTA сервера (ArduinoOTA), TCP.
#define OTA_PORT 3232
/// @brief Пароль OTA (рекомендуется сменить для локальной сети).
#define OTA_PASSWORD "boilerplate"
/// @brief Префикс hostname OTA.
#define OTA_HOSTNAME_PREFIX "boilerplate"

// ===== BLE =====
/// @brief Префикс BLE device name.
#define BLE_DEVICE_NAME_PREFIX "boilerplate"
/// @brief BLE service UUID для runtime-телеметрии.
#define BLE_SERVICE_UUID "35f6f3e0-6e8f-4f19-a6bf-2e5f9b8f4300"
/// @brief BLE characteristic UUID для заряда батареи (%).
#define BLE_CHAR_BATTERY_UUID "35f6f3e1-6e8f-4f19-a6bf-2e5f9b8f4300"
/// @brief BLE characteristic UUID для uptime (сек).
#define BLE_CHAR_UPTIME_UUID "35f6f3e2-6e8f-4f19-a6bf-2e5f9b8f4300"
/// @brief BLE characteristic UUID для версии приложения.
#define BLE_CHAR_VERSION_UUID "35f6f3e3-6e8f-4f19-a6bf-2e5f9b8f4300"
/// @brief Период публикации BLE-телеметрии, миллисекунды.
#define BLE_TELEMETRY_UPDATE_MS 1000
