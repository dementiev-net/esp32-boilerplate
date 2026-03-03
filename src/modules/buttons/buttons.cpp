#include "buttons.h"
#include "../../config.h"

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
    bool lastRaw;
    bool pressed;
    unsigned long pressedAt;
    bool longFired;
};

static ButtonState btnTop    = { BTN_TOP,    false, false, 0, false };
static ButtonState btnBottom = { BTN_BOTTOM, false, false, 0, false };

static void processButton(ButtonState& btn, void (*onClick)(), void (*onLong)()) {
    bool raw = digitalRead(btn.pin) == LOW;

    if (raw && !btn.lastRaw) {
        // Нажатие
        btn.pressed   = true;
        btn.pressedAt = millis();
        btn.longFired = false;
    }

    if (btn.pressed && !btn.longFired) {
        if (millis() - btn.pressedAt >= LONG_PRESS_MS) {
            btn.longFired = true;
            if (onLong) onLong();
        }
    }

    if (!raw && btn.lastRaw) {
        // Отпускание
        if (btn.pressed && !btn.longFired) {
            if (millis() - btn.pressedAt >= DEBOUNCE_MS) {
                if (onClick) onClick();
            }
        }
        btn.pressed = false;
    }

    btn.lastRaw = raw;
}

// ===== PUBLIC =====

void buttonsInit() {
    Serial.println("[Buttons] Initializing...");
    pinMode(BTN_TOP,    INPUT_PULLUP);
    pinMode(BTN_BOTTOM, INPUT_PULLUP);
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