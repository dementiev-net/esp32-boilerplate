#pragma once

#include <Arduino.h>

/**
 * @brief Инициализирует BLE сервер и запускает advertising.
 * @details
 * - Вызывать один раз в `setup()`.
 * - Неблокирующая.
 * - Поднимает GATT service с характеристиками battery/uptime/version.
 * - Если `FEATURE_BLE=0`, функция работает как no-op (модуль отключен профилем сборки).
 * - Не ISR-safe.
 */
void bleInit();

/**
 * @brief Периодическая обработка BLE-телеметрии.
 * @details
 * - Вызывать в каждом проходе `loop()`.
 * - Публикация в характеристики ограничена `BLE_TELEMETRY_UPDATE_MS`.
 * - Если `FEATURE_BLE=0`, функция работает как no-op.
 * - Не ISR-safe.
 */
void bleLoop();

/**
 * @brief Обновляет runtime-телеметрию для BLE.
 * @param batteryPercent Заряд батареи в процентах 0..100, либо `-1` если недоступен.
 * @param uptimeSeconds Uptime устройства в секундах.
 * @details
 * - Неблокирующая.
 * - Новые значения отправляются клиентам из `bleLoop()`.
 * - Если `FEATURE_BLE=0`, значения игнорируются.
 * - Не ISR-safe.
 */
void bleSetTelemetry(int batteryPercent, unsigned long uptimeSeconds);

/**
 * @brief Возвращает состояние BLE-клиента.
 * @return true если есть подключенный BLE-клиент, иначе false.
 * @details При `FEATURE_BLE=0` всегда возвращает false.
 */
bool bleIsConnected();

/**
 * @brief Возвращает готовность BLE сервера.
 * @return true если BLE уже инициализирован и advertising запущен.
 * @details При `FEATURE_BLE=0` всегда возвращает false.
 */
bool bleIsReady();
