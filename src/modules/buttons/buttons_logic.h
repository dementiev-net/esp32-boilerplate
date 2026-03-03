#pragma once

#include <stdint.h>

/**
 * @brief События кнопки, генерируемые state-machine.
 */
enum class ButtonLogicEvent {
    None,
    Click,
    LongPress
};

/**
 * @brief Внутреннее состояние state-machine кнопки.
 */
struct ButtonLogicState {
    bool lastRaw = false;
    bool pressed = false;
    unsigned long pressedAt = 0;
    bool longFired = false;
};

/**
 * @brief Обрабатывает текущее сырье состояние кнопки.
 * @param state Состояние автомата кнопки.
 * @param rawPressed Текущее физическое состояние (true = нажата).
 * @param nowMs Текущее время в миллисекундах.
 * @param debounceMs Время антидребезга в миллисекундах.
 * @param longPressMs Порог long-press в миллисекундах.
 * @return Событие кнопки: Click/LongPress/None.
 */
ButtonLogicEvent buttonsLogicProcess(
    ButtonLogicState& state,
    bool rawPressed,
    unsigned long nowMs,
    unsigned long debounceMs,
    unsigned long longPressMs
);

/**
 * @brief Возвращает текущий флаг удержания кнопки.
 */
bool buttonsLogicIsPressed(const ButtonLogicState& state);
