#pragma once

#include <Arduino.h>

#include "app_dispatcher.h"
#include "runtime_state.h"

/**
 * @brief Результат обработки очереди событий рантайма.
 */
struct AppRuntimeDrainResult {
    bool needUiRefresh = false;
};

/**
 * @brief Инициализирует runtime-диспетчер приложения.
 * @details
 * - Вызывать один раз в `setup()` до публикации событий.
 * - Неблокирующая.
 * - Побочные эффекты: сбрасывает очередь событий runtime.
 * - Не ISR-safe.
 */
void appRuntimeInit();

/**
 * @brief Публикует событие в очередь runtime.
 * @param type Тип события.
 * @return true если событие принято, false при переполнении.
 * @details
 * - Неблокирующая операция очереди.
 * - Побочные эффекты: при успехе увеличивает очередь, при переполнении увеличивает счетчик dropped.
 * - Не ISR-safe.
 */
bool appRuntimePostEvent(AppEventType type);

/**
 * @brief Публикует события изменений между двумя runtime-снимками.
 * @param previous Предыдущий снимок.
 * @param current Текущий снимок.
 * @details
 * - Вызывать из `loop()` после получения нового runtime-снимка.
 * - Неблокирующая.
 * - Побочные эффекты: публикует `...Changed` и `UiRefreshRequested` в очередь.
 * - Не ISR-safe.
 */
void appRuntimePublishStateChanges(
    const RuntimeSnapshot& previous,
    const RuntimeSnapshot& current
);

/**
 * @brief Обрабатывает очередь runtime-событий.
 * @param tracker Трекер runtime (для hold-индикаторов).
 * @param runtimeState Актуальный снимок для диагностических логов.
 * @param nowMs Текущее время, миллисекунды.
 * @return Флаг необходимости обновления UI.
 * @details
 * - Вызывать из `loop()` после публикации событий.
 * - Потенциально блокирующая из-за `Serial.print*` при активном логе.
 * - Побочные эффекты: обновляет hold-индикаторы в `tracker`, пишет диагностику в Serial.
 * - Не ISR-safe.
 */
AppRuntimeDrainResult appRuntimeDrainEvents(
    RuntimeStateTracker& tracker,
    const RuntimeSnapshot& runtimeState,
    unsigned long nowMs
);

/**
 * @brief Возвращает счетчик отброшенных событий dispatcher.
 * @details Неблокирующая, без побочных эффектов.
 */
size_t appRuntimeDroppedEvents();
