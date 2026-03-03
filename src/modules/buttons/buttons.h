#pragma once

#include <Arduino.h>

// Колбэки — назначаются из main.cpp или других модулей
void buttonsInit();
void buttonsLoop();

// Назначение обработчиков
void buttonsOnTopClick(void (*callback)());
void buttonsOnTopLongPress(void (*callback)());
void buttonsOnBottomClick(void (*callback)());
void buttonsOnBottomLongPress(void (*callback)());