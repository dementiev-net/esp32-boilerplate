#pragma once

#include <Arduino.h>

/**
 * @brief Тип активного файлового backend.
 */
enum class StorageFileBackend {
    None,
    SdCard,
    LittleFs
};

/**
 * @brief Инициализирует подсистему хранения: NVS и файловый backend платы.
 * @details
 * - Вызывать один раз из `setup()`.
 * - Для плат с SD (T-Display-S3) поднимается SD backend.
 * - Для плат без SD (T-QT Pro) при `FEATURE_LITTLEFS=1` поднимается LittleFS fallback.
 * - Неблокирующая, но содержит операции монтирования ФС.
 * - Не ISR-safe.
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
 * @brief Возвращает тип активного файлового backend.
 * @return `StorageFileBackend::SdCard`, `StorageFileBackend::LittleFs` или `StorageFileBackend::None`.
 * @details
 * - Вызывать после `storageInit()`.
 * - Неблокирующая.
 * - Не ISR-safe.
 */
StorageFileBackend storageFileBackend();

/**
 * @brief Проверяет доступность активного файлового backend.
 * @return true если backend смонтирован и готов к операциям чтения/записи.
 * @details
 * - Вызывать после `storageInit()`.
 * - Неблокирующая.
 * - Не ISR-safe.
 */
bool storageFileAvailable();

/**
 * @brief Возвращает короткую метку активного файлового backend.
 * @return `"SD"`, `"LittleFS"` или `"None"`.
 * @details
 * - Удобно для логов/диагностики (Serial/UI/shell).
 * - Неблокирующая.
 * - Не ISR-safe.
 */
const char* storageFileBackendLabel();

/**
 * @brief Читает текстовый файл из активного файлового backend.
 * @return Содержимое файла или пустую строку при ошибке/отсутствии backend.
 * @details
 * - На T-Display-S3 читает с SD.
 * - На T-QT Pro при `FEATURE_LITTLEFS=1` читает из LittleFS.
 * - Не ISR-safe.
 */
String sdReadFile(const char* path);

/**
 * @brief Перезаписывает файл целиком (truncate + write) в активном backend.
 * @return true при успешной записи.
 * @details
 * - На T-Display-S3 пишет на SD.
 * - На T-QT Pro при `FEATURE_LITTLEFS=1` пишет в LittleFS.
 * - Не ISR-safe.
 */
bool sdWriteFile(const char* path, const char* content);

/**
 * @brief Дописывает текст в конец файла в активном backend.
 * @return true при успешной записи.
 * @details
 * - На T-Display-S3 пишет на SD.
 * - На T-QT Pro при `FEATURE_LITTLEFS=1` пишет в LittleFS.
 * - Не ISR-safe.
 */
bool sdAppendFile(const char* path, const char* content);
