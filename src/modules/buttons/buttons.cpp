#include "buttons.h"
#include "buttons_logic.h"
#include "../../config.h"
#include "../board/board_profile.h"

// ===== CALLBACKS =====
static void (*onTopClick)() = nullptr;
static void (*onTopLongPress)() = nullptr;
static void (*onTopHold)() = nullptr;
static void (*onBottomClick)() = nullptr;
static void (*onBottomLongPress)() = nullptr;
static void (*onBottomHold)() = nullptr;

struct ButtonState {
    uint8_t pin;
    ButtonLogicState logic;
};

static ButtonState btnTop    = { 0, ButtonLogicState{} };
static ButtonState btnBottom = { 0, ButtonLogicState{} };

static void processButton(ButtonState& btn, void (*onClick)(), void (*onLong)(), void (*onHold)()) {
    bool raw = digitalRead(btn.pin) == LOW;
    ButtonLogicEvent event = buttonsLogicProcess(
        btn.logic,
        raw,
        millis(),
        BUTTON_DEBOUNCE_MS,
        BUTTON_LONG_PRESS_MS,
        BUTTON_HOLD_REPEAT_MS
    );
    if (event == ButtonLogicEvent::LongPress) {
        if (onLong) onLong();
    } else if (event == ButtonLogicEvent::Hold) {
        if (onHold) onHold();
    } else if (event == ButtonLogicEvent::Click) {
        if (onClick) onClick();
    }
}

// ===== PUBLIC =====

void buttonsInit() {
    Serial.println("[Buttons] Initializing...");
    const BoardProfile& board = boardGetProfile();
    btnTop.pin = board.pins.buttonTop;
    btnBottom.pin = board.pins.buttonBottom;
    pinMode(btnTop.pin, INPUT_PULLUP);
    pinMode(btnBottom.pin, INPUT_PULLUP);
    Serial.println("[Buttons] Ready.");
}

void buttonsLoop() {
    processButton(btnTop,    onTopClick,    onTopLongPress,    onTopHold);
    processButton(btnBottom, onBottomClick, onBottomLongPress, onBottomHold);
}

void buttonsOnTopClick(void (*callback)())      { onTopClick      = callback; }
void buttonsOnTopLongPress(void (*callback)())  { onTopLongPress  = callback; }
void buttonsOnTopHold(void (*callback)())       { onTopHold       = callback; }
void buttonsOnBottomClick(void (*callback)())   { onBottomClick   = callback; }
void buttonsOnBottomLongPress(void (*callback)()) { onBottomLongPress = callback; }
void buttonsOnBottomHold(void (*callback)())    { onBottomHold    = callback; }

bool buttonsIsTopPressed() {
    return buttonsLogicIsPressed(btnTop.logic);
}

bool buttonsIsBottomPressed() {
    return buttonsLogicIsPressed(btnBottom.logic);
}
