#include "buttons_logic.h"

ButtonLogicEvent buttonsLogicProcess(
    ButtonLogicState& state,
    bool rawPressed,
    unsigned long nowMs,
    unsigned long debounceMs,
    unsigned long longPressMs,
    unsigned long holdRepeatMs
) {
    if (rawPressed && !state.lastRaw) {
        state.pressed = true;
        state.pressedAt = nowMs;
        state.longFired = false;
        state.lastHoldAt = 0;
    }

    if (state.pressed && !state.longFired) {
        if (nowMs - state.pressedAt >= longPressMs) {
            state.longFired = true;
            state.lastHoldAt = nowMs;
            state.lastRaw = rawPressed;
            return ButtonLogicEvent::LongPress;
        }
    }

    if (state.pressed && state.longFired && holdRepeatMs > 0) {
        if (nowMs - state.lastHoldAt >= holdRepeatMs) {
            state.lastHoldAt = nowMs;
            state.lastRaw = rawPressed;
            return ButtonLogicEvent::Hold;
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
        state.lastHoldAt = 0;
    }

    state.lastRaw = rawPressed;
    return ButtonLogicEvent::None;
}

bool buttonsLogicIsPressed(const ButtonLogicState& state) {
    return state.pressed;
}
