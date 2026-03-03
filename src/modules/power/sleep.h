#pragma once

#include <stdint.h>

/**
 * @brief Инициализирует power-sleep подсистему и определяет причину пробуждения.
 * @details
 * - Вызывать один раз в `setup()`.
 * - Неблокирующая (кроме конфигурации wakeup-источника).
 * - Побочные эффекты: настраивает wakeup по верхней кнопке (если поддерживается).
 * - Ограничения по платам: wakeup доступен только для RTC GPIO.
 * - Не ISR-safe.
 */
void sleepInit();

/**
 * @brief Возвращает признак поддержки пробуждения по кнопке.
 * @return true если wakeup-пин успешно сконфигурирован.
 */
bool sleepCanWakeByButton();

/**
 * @brief Возвращает GPIO кнопки пробуждения.
 * @return Номер GPIO или `-1`, если не сконфигурировано.
 */
int sleepWakeButtonPin();

/**
 * @brief Возвращает строковую причину последнего пробуждения.
 * @return Короткий текст причины (`cold_boot`, `button_wakeup`, и т.д.).
 */
const char* sleepWakeReasonLabel();

/**
 * @brief Переводит контроллер в deep sleep.
 * @details
 * - Блокирующая, не возвращается при успешном входе в сон.
 * - Перед сном выводит диагностический лог в Serial.
 * - Не ISR-safe.
 */
void sleepEnterDeepSleep();
