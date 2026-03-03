#pragma once

#include <Arduino.h>

/**
 * @brief Инициализирует модуль надежности: reset-diagnostics и task watchdog.
 * @details
 * - Вызывать один раз в `setup()` сразу после `Serial.begin(...)`.
 * - Потенциально блокирующая на время инициализации WDT драйвера.
 * - Побочные эффекты: регистрирует текущую loop-задачу в watchdog, пишет диагностику в Serial.
 * - Поведение управляется `FEATURE_WATCHDOG` и `WATCHDOG_TIMEOUT_SEC` из `config.h`.
 * - Не ISR-safe.
 */
void reliabilityInit();

/**
 * @brief Периодический feed watchdog для loop-задачи.
 * @details
 * - Вызывать в каждом проходе `loop()`.
 * - Неблокирующая.
 * - Побочные эффекты: сбрасывает watchdog-таймер с периодом `WATCHDOG_FEED_INTERVAL_MS`.
 * - При `FEATURE_WATCHDOG=0` работает как no-op.
 * - Не ISR-safe.
 */
void reliabilityLoop();

/**
 * @brief Возвращает признак активного watchdog для loop-задачи.
 * @return true если watchdog успешно инициализирован и задача зарегистрирована.
 * @details Неблокирующая, без побочных эффектов.
 */
bool reliabilityWatchdogEnabled();

/**
 * @brief Возвращает строковую причину последнего reset.
 * @return Короткая метка (`power_on`, `task_wdt`, `panic`, `deep_sleep`, ...).
 * @details
 * - Обновляется в `reliabilityInit()`.
 * - Неблокирующая, без побочных эффектов.
 */
const char* reliabilityResetReasonLabel();

/**
 * @brief Возвращает числовой код причины reset (`esp_reset_reason_t`).
 * @return Целочисленный код reset reason.
 * @details
 * - Обновляется в `reliabilityInit()`.
 * - Неблокирующая, без побочных эффектов.
 */
int reliabilityResetReasonCode();

/**
 * @brief Возвращает uptime устройства в секундах.
 * @return Количество секунд с момента старта MCU.
 * @details Неблокирующая, без побочных эффектов.
 */
unsigned long reliabilityUptimeSeconds();
