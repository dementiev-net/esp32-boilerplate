#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

void displayInit();

// Очистка экрана
void displayClear(uint32_t color = 0x0000);

// Текст
void displayPrint(int x, int y, const char* text, uint32_t color = 0xFFFF, uint8_t size = 1);

// Примитивы
void displayDrawRect(int x, int y, int w, int h, uint32_t color);
void displayFillRect(int x, int y, int w, int h, uint32_t color);
void displayDrawLine(int x1, int y1, int x2, int y2, uint32_t color);

// Статус-бар
void displayStatusBar(const char* left, const char* right);