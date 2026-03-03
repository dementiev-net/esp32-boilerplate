#pragma once

// ===== VERSION =====
/// @brief Версия приложения для вывода на экран/в логи.
#define APP_VERSION "1.1.0"
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

// ===== DISPLAY =====
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
  #define SCREEN_ROTATION 0
  /// @brief Пин включения питания 5V для платы T-QT Pro.
  #define PIN_ENABLE5V    4
#endif

// ===== BUTTONS =====
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
