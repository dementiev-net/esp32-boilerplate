#pragma once

#include <Arduino.h>

/**
 * @brief Инициализирует модуль измерения VBAT через ADC-пин текущей платы.
 * @details
 * - Вызывать один раз в `setup()` после выбора board profile.
 * - Неблокирующая (кроме краткого доступа к API ADC).
 * - Побочные эффекты: настраивает ADC-разрешение/attenuation и пин входа.
 * - Ограничения по платам: работает только если `board.pins.batteryAdcPin >= 0`.
 * - Не ISR-safe.
 */
void batteryInit();

/**
 * @brief Возвращает поддержку измерения батареи на текущей плате.
 * @return true если в board profile задан ADC-пин VBAT.
 * @details Неблокирующая, без побочных эффектов.
 */
bool batteryIsSupported();

/**
 * @brief Возвращает признак активного USB CDC-хоста.
 * @return true если USB CDC активен (подключение к ПК/Serial Monitor).
 * @details
 * - Используется как эвристика для UI: при USB можно скрывать проценты батареи.
 * - На ESP32-S3 USB Serial JTAG определяется по SOF-фреймам USB-хоста.
 * - Неблокирующая, без побочных эффектов.
 * - Ограничения по платам: зависит от реализации USB CDC в Arduino core.
 */
bool batteryUsbHostConnected();

/**
 * @brief Считывает текущее напряжение батареи в милливольтах.
 * @return Напряжение в mV или `-1`, если значение невалидно/батарея не обнаружена.
 * @details
 * - Допустимо вызывать из `loop()` с периодом, заданным в runtime.
 * - Потенциально блокирующая на время нескольких ADC-чтений.
 * - Применяет коэффициент делителя из `config.h`.
 * - Не ISR-safe.
 */
int batteryReadMillivolts();

/**
 * @brief Конвертирует напряжение батареи в процент заряда.
 * @param millivolts Напряжение батареи, милливольты.
 * @return Процент 0..100 или `-1`, если вход невалиден.
 * @details
 * - Использует линейную шкалу `BATTERY_PERCENT_EMPTY_MV..BATTERY_PERCENT_FULL_MV`.
 * - Неблокирующая, без побочных эффектов.
 */
int batteryMillivoltsToPercent(int millivolts);
