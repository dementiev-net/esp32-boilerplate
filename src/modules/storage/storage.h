#pragma once

#include <Arduino.h>

/**
 * @brief Инициализирует подсистему хранения: NVS и SD (если поддерживается платой).
 * @details Вызывать один раз из setup(). Не ISR-safe.
 */
void storageInit();

/**
 * @brief Проверяет доступность NVS после инициализации.
 * @return true если Preferences успешно открыты.
 */
bool storageNvsAvailable();

/**
 * @brief Сохраняет строку в NVS по ключу.
 * @note Если NVS не инициализирован, операция игнорируется.
 */
void storageSetString(const char* key, const char* value);

/**
 * @brief Читает строку из NVS.
 * @return Значение по ключу или defaultValue.
 */
String storageGetString(const char* key, const char* defaultValue = "");

/**
 * @brief Сохраняет целое число в NVS.
 */
void storageSetInt(const char* key, int value);

/**
 * @brief Читает целое число из NVS.
 * @return Значение по ключу или defaultValue.
 */
int storageGetInt(const char* key, int defaultValue = 0);

/**
 * @brief Возвращает, поддерживает ли текущая плата SD в этом проекте.
 */
bool sdSupported();

/**
 * @brief Возвращает, смонтирована ли SD-карта.
 */
bool sdAvailable();

/**
 * @brief Читает текстовый файл с SD.
 * @return Содержимое файла или пустую строку при ошибке.
 */
String sdReadFile(const char* path);

/**
 * @brief Перезаписывает файл целиком (truncate + write).
 * @return true при успешной записи.
 */
bool sdWriteFile(const char* path, const char* content);

/**
 * @brief Дописывает текст в конец файла.
 * @return true при успешной записи.
 */
bool sdAppendFile(const char* path, const char* content);
