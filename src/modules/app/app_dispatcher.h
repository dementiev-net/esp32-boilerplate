#pragma once

#include <Arduino.h>
#include <stdint.h>

/**
 * @brief Типы событий рантайма приложения.
 */
enum class AppEventType : uint8_t {
    None = 0,
    ButtonTopClick,
    ButtonTopLongPress,
    ButtonTopHold,
    ButtonBottomClick,
    ButtonBottomLongPress,
    ButtonBottomHold,
    ButtonsStateChanged,
    WifiConnectionChanged,
    WifiIdentityChanged,
    SdStateChanged,
    TimeUpdated,
    NetUpdated,
    BatteryUpdated,
    BrightnessUpdated,
    UiRefreshRequested
};

/**
 * @brief Событие в очереди dispatcher.
 */
struct AppEvent {
    AppEventType type = AppEventType::None;
    unsigned long timestampMs = 0;
};

/**
 * @brief Сбрасывает внутреннее состояние dispatcher и очищает очередь.
 * @details Вызывать один раз в setup() до публикации событий.
 */
void appDispatcherInit();

/**
 * @brief Публикует событие в очередь dispatcher.
 * @param type Тип публикуемого события.
 * @return true если событие добавлено, false при переполнении очереди.
 * @note Не ISR-safe.
 */
bool appDispatcherPost(AppEventType type);

/**
 * @brief Читает следующее событие из очереди.
 * @param out Структура для результата.
 * @return true если событие получено, false если очередь пуста.
 */
bool appDispatcherNext(AppEvent& out);

/**
 * @brief Возвращает число отброшенных событий при переполнении.
 */
size_t appDispatcherDroppedCount();
