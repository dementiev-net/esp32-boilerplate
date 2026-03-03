#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

/**
 * @brief Инициализирует дисплей и подсветку.
 * @details Вызывать один раз из setup(). Не ISR-safe.
 * @note Применяет rotation и board-specific пины из `board_profile`.
 */
void displayInit();

/**
 * @brief Полностью очищает экран заданным цветом.
 * @param color Цвет в формате TFT_eSPI (RGB565).
 * @warning Блокирующая операция; избегать частого вызова в loop().
 */
void displayClear(uint32_t color = 0x0000);

/**
 * @brief Рисует строку текста в координатах экрана.
 * @param x Координата X (пиксели).
 * @param y Координата Y (пиксели).
 * @param text Нуль-терминированная строка.
 * @param color Цвет текста RGB565.
 * @param size Масштаб шрифта TFT_eSPI.
 * @note Не ISR-safe.
 */
void displayPrint(int x, int y, const char* text, uint32_t color = 0xFFFF, uint8_t size = 1);

/**
 * @brief Рисует контур прямоугольника.
 */
void displayDrawRect(int x, int y, int w, int h, uint32_t color);
/**
 * @brief Рисует залитый прямоугольник.
 */
void displayFillRect(int x, int y, int w, int h, uint32_t color);
/**
 * @brief Рисует линию между двумя точками.
 */
void displayDrawLine(int x1, int y1, int x2, int y2, uint32_t color);

/**
 * @brief Рисует простую строку статуса с левым и правым текстом.
 * @param left Текст слева.
 * @param right Текст справа.
 */
void displayStatusBar(const char* left, const char* right);

/**
 * @brief Возвращает поддержку регулировки яркости подсветки.
 * @return true если на текущей плате доступно PWM-управление подсветкой.
 * @details
 * - Можно вызывать из `setup()`/`loop()` после `displayInit()`.
 * - Неблокирующая, без побочных эффектов.
 * - Ограничения по платам: зависит от наличия `TFT_BL` в board-specific setup.
 */
bool displayBrightnessSupported();

/**
 * @brief Возвращает текущую яркость подсветки в процентах.
 * @return Яркость `0..100`.
 * @details
 * - Можно вызывать из `setup()`/`loop()` после `displayInit()`.
 * - Неблокирующая, без побочных эффектов.
 * - Если PWM недоступен, возвращает последнее примененное значение по умолчанию.
 */
uint8_t displayGetBrightnessPercent();

/**
 * @brief Устанавливает яркость подсветки в процентах.
 * @param percent Яркость `0..100`.
 * @return true если значение применено, false если управление яркостью недоступно.
 * @details
 * - Вызывать из `setup()`/`loop()` по событию UI/shell.
 * - Неблокирующая (запись в регистр PWM).
 * - Побочные эффекты: меняет duty PWM подсветки дисплея (GPIO `TFT_BL`).
 * - Ограничения по платам: работает только при наличии `TFT_BL` и активного LEDC.
 * - Не ISR-safe.
 */
bool displaySetBrightnessPercent(uint8_t percent);
