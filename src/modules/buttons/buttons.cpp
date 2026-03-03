#include "buttons.h"
#include "buttons_logic.h"
#include "../board/board_profile.h"

// ===== CALLBACKS =====
static void (*onTopClick)() = nullptr;
static void (*onTopLongPress)() = nullptr;
static void (*onBottomClick)() = nullptr;
static void (*onBottomLongPress)() = nullptr;

// ===== DEBOUNCE =====
#define DEBOUNCE_MS   50
#define LONG_PRESS_MS 800

struct ButtonState {
    uint8_t pin;
    ButtonLogicState logic;
};

static ButtonState btnTop    = { 0, ButtonLogicState{} };
static ButtonState btnBottom = { 0, ButtonLogicState{} };

static void processButton(ButtonState& btn, void (*onClick)(), void (*onLong)()) {
    bool raw = digitalRead(btn.pin) == LOW;
    ButtonLogicEvent event = buttonsLogicProcess(btn.logic, raw, millis(), DEBOUNCE_MS, LONG_PRESS_MS);
    if (event == ButtonLogicEvent::LongPress) {
        if (onLong) onLong();
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
    processButton(btnTop,    onTopClick,    onTopLongPress);
    processButton(btnBottom, onBottomClick, onBottomLongPress);
}

void buttonsOnTopClick(void (*callback)())      { onTopClick      = callback; }
void buttonsOnTopLongPress(void (*callback)())  { onTopLongPress  = callback; }
void buttonsOnBottomClick(void (*callback)())   { onBottomClick   = callback; }
void buttonsOnBottomLongPress(void (*callback)()) { onBottomLongPress = callback; }

bool buttonsIsTopPressed() {
    return buttonsLogicIsPressed(btnTop.logic);
}

bool buttonsIsBottomPressed() {
    return buttonsLogicIsPressed(btnBottom.logic);
}
