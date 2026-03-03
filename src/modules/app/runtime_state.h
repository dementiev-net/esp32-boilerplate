#pragma once

#include <Arduino.h>

/**
 * @brief Снимок runtime-параметров для UI и dispatcher.
 */
struct RuntimeSnapshot {
    bool topPressed = false;
    bool bottomPressed = false;
    bool topHoldActive = false;
    bool bottomHoldActive = false;
    bool wifiConnected = false;
    bool sdSupported = false;
    bool sdAvailable = false;
    String modeLabel = "";
    String ip = "-";
    String ssid = "";
    bool timeSynced = false;
    String localTime = "--:--:--";
};

/**
 * @brief Внутреннее состояние трекера runtime.
 * @details
 * - Содержит тайм-ауты hold-индикаторов.
 * - Хранит кэш отображаемого времени для ограничения redraw до 1 Гц.
 */
struct RuntimeStateTracker {
    unsigned long topHoldUntilMs = 0;
    unsigned long bottomHoldUntilMs = 0;
    String cachedUiTime = "--:--:--";
    unsigned long lastTimeUiSampleMs = 0;
};

/**
 * @brief Инициализирует трекер runtime-состояния.
 * @param tracker Трекер состояния.
 * @details
 * - Вызывать один раз в `setup()` до первого `runtimeStateRead`.
 * - Неблокирующая (меняет только поля в RAM, без I/O).
 * - Не ISR-safe.
 */
void runtimeStateInit(RuntimeStateTracker& tracker);

/**
 * @brief Активирует hold-индикатор верхней кнопки.
 * @param tracker Трекер состояния.
 * @param nowMs Текущее время, миллисекунды.
 * @param indicatorMs Длительность индикации hold, миллисекунды.
 * @details
 * - Неблокирующая.
 * - Побочный эффект: обновляет deadline индикации в RAM.
 * - Не ISR-safe.
 */
void runtimeStateActivateTopHold(RuntimeStateTracker& tracker, unsigned long nowMs, unsigned long indicatorMs);

/**
 * @brief Активирует hold-индикатор нижней кнопки.
 * @param tracker Трекер состояния.
 * @param nowMs Текущее время, миллисекунды.
 * @param indicatorMs Длительность индикации hold, миллисекунды.
 * @details
 * - Неблокирующая.
 * - Побочный эффект: обновляет deadline индикации в RAM.
 * - Не ISR-safe.
 */
void runtimeStateActivateBottomHold(RuntimeStateTracker& tracker, unsigned long nowMs, unsigned long indicatorMs);

/**
 * @brief Считывает текущий runtime-снимок из модулей.
 * @param tracker Трекер состояния.
 * @param nowMs Текущее время, миллисекунды.
 * @return Полный снимок для UI/dispatcher.
 * @details
 * - Вызывать из `loop()` (обычно каждый проход).
 * - Неблокирующая в рамках текущей реализации (опрос состояния модулей и RAM-кэша).
 * - Побочные эффекты: обновляет внутренний кэш времени (`cachedUiTime`, `lastTimeUiSampleMs`).
 * - Для TIME обновляет кэш не чаще 1 раза в секунду (1 Hz).
 * - Ограничения по платам: поле `sdSupported` зависит от board profile (`false` на T-QT Pro).
 * - Не ISR-safe.
 */
RuntimeSnapshot runtimeStateRead(RuntimeStateTracker& tracker, unsigned long nowMs);

/**
 * @brief Сравнивает два runtime-снимка.
 * @return true если снимки идентичны.
 * @details
 * - Неблокирующая, чистая функция (без I/O и побочных эффектов).
 * - Допустимо вызывать в `loop()` без ограничений по частоте.
 */
bool runtimeStateEquals(const RuntimeSnapshot& a, const RuntimeSnapshot& b);
