#pragma once

#include <stdint.h>

/**
 * @brief События кнопки, генерируемые state-machine.
 */
enum class ButtonLogicEvent {
    None,
    Click,
    LongPress,
    Hold
};

/**
 * @brief Внутреннее состояние state-machine кнопки.
 */
struct ButtonLogicState {
    bool lastRaw = false;
    bool pressed = false;
    unsigned long pressedAt = 0;
    bool longFired = false;
    unsigned long lastHoldAt = 0;
};

/**
 * @brief Обрабатывает текущее сырье состояние кнопки.
 * @param state Состояние автомата кнопки.
 * @param rawPressed Текущее физическое состояние (true = нажата).
 * @param nowMs Текущее время в миллисекундах.
 * @param debounceMs Время антидребезга в миллисекундах.
 * @param longPressMs Порог long-press в миллисекундах.
 * @param holdRepeatMs Интервал генерации события Hold после LongPress, миллисекунды.
 * @return Событие кнопки: Click/LongPress/Hold/None.
 */
ButtonLogicEvent buttonsLogicProcess(
    ButtonLogicState& state,
    bool rawPressed,
    unsigned long nowMs,
    unsigned long debounceMs,
    unsigned long longPressMs,
    unsigned long holdRepeatMs
);

/**
 * @brief Возвращает текущий флаг удержания кнопки.
 */
bool buttonsLogicIsPressed(const ButtonLogicState& state);
