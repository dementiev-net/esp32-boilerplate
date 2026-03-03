#pragma once

#include <Arduino.h>

/**
 * @brief Инициализирует GPIO кнопок.
 * @details Вызывать один раз из setup(). Не ISR-safe.
 */
void buttonsInit();

/**
 * @brief Обновляет state-machine кнопок и вызывает колбэки событий.
 * @details Вызывать регулярно из loop().
 */
void buttonsLoop();

/**
 * @brief Назначает обработчик клика верхней кнопки.
 */
void buttonsOnTopClick(void (*callback)());
/**
 * @brief Назначает обработчик долгого нажатия верхней кнопки.
 */
void buttonsOnTopLongPress(void (*callback)());
/**
 * @brief Назначает обработчик клика нижней кнопки.
 */
void buttonsOnBottomClick(void (*callback)());
/**
 * @brief Назначает обработчик долгого нажатия нижней кнопки.
 */
void buttonsOnBottomLongPress(void (*callback)());

/**
 * @brief Возвращает текущее состояние верхней кнопки.
 * @return true если кнопка сейчас удерживается.
 */
bool buttonsIsTopPressed();

/**
 * @brief Возвращает текущее состояние нижней кнопки.
 * @return true если кнопка сейчас удерживается.
 */
bool buttonsIsBottomPressed();
