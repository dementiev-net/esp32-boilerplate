#pragma once

#include <stdint.h>

/**
 * @brief Инициализирует и рисует boot-preloader (логотип + рамка прогресса).
 * @details
 * - Вызывать один раз в `setup()` после `displayInit()`.
 * - Потенциально блокирующая на время первичной отрисовки.
 * - Побочные эффекты: очищает экран и рисует статический слой preloader.
 * - Ограничения по платам: размеры/позиции адаптируются под текущий board profile.
 * - Не ISR-safe.
 */
void bootPreloaderBegin();

/**
 * @brief Обновляет этап и прогресс boot-preloader.
 * @param progressPercent Прогресс 0..100.
 * @param stageLabel Текст этапа.
 * @param smooth true для плавного дотягивания до нового процента.
 * @details
 * - Перерисовывает только динамические области preloader.
 * - Потенциально блокирующая: при `smooth=true` использует короткие `delay(5 ms)`.
 * - Побочные эффекты: обновляет текст этапа/прогресс на экране.
 * - Не ISR-safe.
 */
void bootPreloaderStep(uint8_t progressPercent, const char* stageLabel, bool smooth = true);

/**
 * @brief Завершает preloader с учетом минимального времени показа.
 * @details
 * - Гарантирует отображение заставки не менее `BOOT_SPLASH_MIN_MS`.
 * - Блокирующая до достижения минимальной длительности заставки.
 * - Единицы: `BOOT_SPLASH_MIN_MS` в миллисекундах.
 * - Показывает короткую анимацию завершения.
 * - Побочные эффекты: обновляет stage-строку preloader.
 * - Не ISR-safe.
 */
void bootPreloaderFinish();
