#pragma once

#include <Arduino.h>

// Инициализация (NVS + SD карта)
void storageInit();

// NVS — сохранение/чтение строк
void storageSetString(const char* key, const char* value);
String storageGetString(const char* key, const char* defaultValue = "");

// NVS — сохранение/чтение чисел
void storageSetInt(const char* key, int value);
int storageGetInt(const char* key, int defaultValue = 0);

// SD карта
bool sdAvailable();
String sdReadFile(const char* path);
bool sdWriteFile(const char* path, const char* content);