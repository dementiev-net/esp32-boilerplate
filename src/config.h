#pragma once

// ===== VERSION =====
#define APP_VERSION "1.0.0"
#define APP_NAME    "Boilerplate"

// ===== SERIAL =====
#define SERIAL_BAUD_RATE 115200

// ===== WIFI =====
#define WIFI_AP_NAME     "ESP32-Boilerplate"
#define WIFI_AP_PASSWORD "12345678"
#define WIFI_TIMEOUT_MS  10000

// ===== DISPLAY =====
#ifdef BOARD_TDISPLAY_S3
  #define SCREEN_WIDTH    320
  #define SCREEN_HEIGHT   170
  #define SCREEN_ROTATION 1
#endif

#ifdef BOARD_TQT_PRO
  #define SCREEN_WIDTH    128
  #define SCREEN_HEIGHT   128
  #define SCREEN_ROTATION 0
  #define PIN_ENABLE5V    4
#endif

// ===== BUTTONS =====
#ifdef BOARD_TDISPLAY_S3
  #define BTN_TOP    14
  #define BTN_BOTTOM  0
#endif

#ifdef BOARD_TQT_PRO
  #define BTN_TOP     0
  #define BTN_BOTTOM 47
#endif

// ===== SD CARD =====
#ifdef BOARD_TDISPLAY_S3
  #define SD_CS   13
  #define SD_MOSI 11
  #define SD_MISO 12
  #define SD_SCK  10
#endif

// ===== STORAGE =====
#define NVS_NAMESPACE "boilerplate"