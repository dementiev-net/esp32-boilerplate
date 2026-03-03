#pragma once

#include <Arduino.h>

/**
 * @brief Инициализирует demo HTTP-клиент для чтения JSON из интернета.
 * @details
 * - Вызывать один раз из `setup()` после инициализации Wi-Fi.
 * - Неблокирующая.
 * - Побочные эффекты: сбрасывает внутреннее состояние last value/error в RAM.
 * - При `FEATURE_NET_HTTP=0` работает как no-op.
 * - Не ISR-safe.
 */
void netDemoInit();

/**
 * @brief Периодически выполняет HTTP-запрос к demo endpoint и обновляет кэш результата.
 * @param internetAvailable true, если доступен интернет (STA подключен, не AP-only).
 * @details
 * - Вызывать из `loop()` с любой частотой.
 * - Неблокирующая: сетевой запрос выполняется в фоновой FreeRTOS-задаче.
 * - Побочные эффекты: сетевой трафик, обновление внутренних кэшей, вывод диагностики в Serial.
 * - Интервал запросов задается `NET_DEMO_REQUEST_INTERVAL_MS`.
 * - При `FEATURE_NET_HTTP=0` работает как no-op.
 * - Не ISR-safe.
 */
void netDemoLoop(bool internetAvailable);

/**
 * @brief Возвращает, включен ли модуль demo HTTP клиента на этапе компиляции.
 * @details Неблокирующая, без побочных эффектов.
 */
bool netDemoIsEnabled();

/**
 * @brief Возвращает, была ли последняя попытка запроса успешной.
 * @details
 * - Неблокирующая, без побочных эффектов.
 * - Если запросов еще не было, возвращает false.
 */
bool netDemoLastRequestOk();

/**
 * @brief Возвращает последнее успешно полученное значение из JSON.
 * @return Строка значения (например внешний IP) или `"-"` при отсутствии данных.
 * @details Неблокирующая, без побочных эффектов.
 */
String netDemoGetValue();

/**
 * @brief Возвращает короткий статус для UI (например `NET:...`, `NET:ERR`, `NET:-`).
 * @details Неблокирующая, без побочных эффектов.
 */
String netDemoGetUiText();

/**
 * @brief Возвращает время последней попытки запроса.
 * @return Миллисекунды `millis()` либо 0, если попыток еще не было.
 * @details Неблокирующая, без побочных эффектов.
 */
unsigned long netDemoLastAttemptMs();

/**
 * @brief Возвращает время последнего успешного ответа.
 * @return Миллисекунды `millis()` либо 0, если успешного ответа еще не было.
 * @details Неблокирующая, без побочных эффектов.
 */
unsigned long netDemoLastSuccessMs();
