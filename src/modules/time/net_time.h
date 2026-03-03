#pragma once

#include <Arduino.h>

/**
 * @brief Инициализирует состояние NTP-синхронизации времени.
 * @details Вызывать один раз из setup(). Не ISR-safe.
 */
void netTimeInit();

/**
 * @brief Периодическая обработка NTP-синхронизации.
 * @param internetAvailable true если есть подключение к интернету (не AP-only режим).
 * @details
 * - Вызывать регулярно из loop().
 * - При наличии интернета запускает/повторяет NTP-синхронизацию.
 */
void netTimeLoop(bool internetAvailable);

/**
 * @brief Возвращает признак валидного синхронизированного времени.
 */
bool netTimeIsSynced();

/**
 * @brief Возвращает локальное время в формате `HH:MM:SS`.
 * @return `--:--:--`, если время еще не синхронизировано.
 */
String netTimeGetLocalTimeText();
