#pragma once

// ===== VERSION =====
/// @brief Версия приложения для вывода на экран/в логи.
#define APP_VERSION "1.2.5"
/// @brief Короткое имя приложения для UI/логов.
#define APP_NAME    "Boilerplate"

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
/// @brief Напряжение пустой Li-ion батареи для расчета процента, милливольты.
#define BATTERY_PERCENT_EMPTY_MV 3300
/// @brief Напряжение полной Li-ion батареи для расчета процента, милливольты.
#define BATTERY_PERCENT_FULL_MV 4200

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
