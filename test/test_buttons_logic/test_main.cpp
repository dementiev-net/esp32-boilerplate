#include <unity.h>

#include "modules/buttons/buttons_logic.h"

void test_click_emits_on_release_after_debounce() {
    ButtonLogicState state;

    ButtonLogicEvent e1 = buttonsLogicProcess(state, true, 100, 50, 800, 250);
    ButtonLogicEvent e2 = buttonsLogicProcess(state, true, 130, 50, 800, 250);
    ButtonLogicEvent e3 = buttonsLogicProcess(state, false, 170, 50, 800, 250);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(ButtonLogicEvent::None), static_cast<int>(e1));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(ButtonLogicEvent::None), static_cast<int>(e2));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(ButtonLogicEvent::Click), static_cast<int>(e3));
    TEST_ASSERT_FALSE(buttonsLogicIsPressed(state));
}

void test_long_press_emits_once() {
    ButtonLogicState state;

    ButtonLogicEvent e1 = buttonsLogicProcess(state, true, 0, 50, 800, 250);
    ButtonLogicEvent e2 = buttonsLogicProcess(state, true, 810, 50, 800, 250);
    ButtonLogicEvent e3 = buttonsLogicProcess(state, true, 900, 50, 800, 250);
    ButtonLogicEvent e4 = buttonsLogicProcess(state, false, 920, 50, 800, 250);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(ButtonLogicEvent::None), static_cast<int>(e1));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(ButtonLogicEvent::LongPress), static_cast<int>(e2));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(ButtonLogicEvent::None), static_cast<int>(e3));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(ButtonLogicEvent::None), static_cast<int>(e4));
}

void test_short_bounce_does_not_emit_click() {
    ButtonLogicState state;

    ButtonLogicEvent e1 = buttonsLogicProcess(state, true, 1000, 50, 800, 250);
    ButtonLogicEvent e2 = buttonsLogicProcess(state, false, 1020, 50, 800, 250);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(ButtonLogicEvent::None), static_cast<int>(e1));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(ButtonLogicEvent::None), static_cast<int>(e2));
    TEST_ASSERT_FALSE(buttonsLogicIsPressed(state));
}

void test_hold_emits_periodically_after_long_press() {
    ButtonLogicState state;

    ButtonLogicEvent e1 = buttonsLogicProcess(state, true, 0, 50, 800, 250);
    ButtonLogicEvent e2 = buttonsLogicProcess(state, true, 810, 50, 800, 250);
    ButtonLogicEvent e3 = buttonsLogicProcess(state, true, 900, 50, 800, 250);
    ButtonLogicEvent e4 = buttonsLogicProcess(state, true, 1065, 50, 800, 250);
    ButtonLogicEvent e5 = buttonsLogicProcess(state, true, 1315, 50, 800, 250);
    ButtonLogicEvent e6 = buttonsLogicProcess(state, false, 1400, 50, 800, 250);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(ButtonLogicEvent::None), static_cast<int>(e1));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(ButtonLogicEvent::LongPress), static_cast<int>(e2));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(ButtonLogicEvent::None), static_cast<int>(e3));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(ButtonLogicEvent::Hold), static_cast<int>(e4));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(ButtonLogicEvent::Hold), static_cast<int>(e5));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(ButtonLogicEvent::None), static_cast<int>(e6));
    TEST_ASSERT_FALSE(buttonsLogicIsPressed(state));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_click_emits_on_release_after_debounce);
    RUN_TEST(test_long_press_emits_once);
    RUN_TEST(test_short_bounce_does_not_emit_click);
    RUN_TEST(test_hold_emits_periodically_after_long_press);
    return UNITY_END();
}
