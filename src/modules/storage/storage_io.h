#pragma once

#include <stdint.h>

/**
 * @brief Режим записи файла.
 */
enum class StorageWriteMode {
    Overwrite,
    Append
};

/**
 * @brief Результат операции записи в абстрактное хранилище.
 */
enum class StorageIoResult {
    Ok,
    InvalidArgs,
    TruncateFailed,
    OpenFailed,
    WriteFailed
};

/**
 * @brief Набор callback-функций файловой операции для тестируемой записи.
 * @details Позволяет тестировать логику write/append без реальной SD/FS.
 */
struct StorageIo {
    bool (*exists)(void* ctx, const char* path);
    bool (*remove)(void* ctx, const char* path);
    bool (*openWrite)(void* ctx, const char* path, bool append);
    bool (*writeText)(void* ctx, const char* content);
    void (*close)(void* ctx);
    void* ctx;
};

/**
 * @brief Унифицированная запись текста с поддержкой overwrite/append.
 * @param io Таблица операций файловой системы.
 * @param path Путь к файлу.
 * @param content Нуль-терминированный текст.
 * @param mode Режим записи.
 * @return Код результата операции.
 */
StorageIoResult storageWriteText(StorageIo* io, const char* path, const char* content, StorageWriteMode mode);
