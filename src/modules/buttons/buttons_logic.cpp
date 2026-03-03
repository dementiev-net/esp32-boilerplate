#include "buttons_logic.h"

ButtonLogicEvent buttonsLogicProcess(
    ButtonLogicState& state,
    bool rawPressed,
    unsigned long nowMs,
    unsigned long debounceMs,
    unsigned long longPressMs
) {
    if (rawPressed && !state.lastRaw) {
        state.pressed = true;
        state.pressedAt = nowMs;
        state.longFired = false;
    }

    if (state.pressed && !state.longFired) {
        if (nowMs - state.pressedAt >= longPressMs) {
            state.longFired = true;
            state.lastRaw = rawPressed;
            return ButtonLogicEvent::LongPress;
        }
    }

    if (!rawPressed && state.lastRaw) {
        if (state.pressed && !state.longFired) {
            if (nowMs - state.pressedAt >= debounceMs) {
                state.pressed = false;
                state.lastRaw = rawPressed;
                return ButtonLogicEvent::Click;
            }
        }
        state.pressed = false;
    }

    state.lastRaw = rawPressed;
    return ButtonLogicEvent::None;
}

bool buttonsLogicIsPressed(const ButtonLogicState& state) {
    return state.pressed;
}
