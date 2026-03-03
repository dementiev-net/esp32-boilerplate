#include "storage_io.h"

#include <cstring>

static bool invalidInput(StorageIo* io, const char* path, const char* content) {
    return io == nullptr
        || io->exists == nullptr
        || io->remove == nullptr
        || io->openWrite == nullptr
        || io->writeText == nullptr
        || io->close == nullptr
        || path == nullptr
        || std::strlen(path) == 0
        || content == nullptr;
}

StorageIoResult storageWriteText(StorageIo* io, const char* path, const char* content, StorageWriteMode mode) {
    if (invalidInput(io, path, content)) {
        return StorageIoResult::InvalidArgs;
    }

    const bool append = (mode == StorageWriteMode::Append);

    if (!append && io->exists(io->ctx, path)) {
        if (!io->remove(io->ctx, path)) {
            return StorageIoResult::TruncateFailed;
        }
    }

    if (!io->openWrite(io->ctx, path, append)) {
        return StorageIoResult::OpenFailed;
    }

    const bool writeOk = io->writeText(io->ctx, content);
    io->close(io->ctx);
    if (!writeOk) {
        return StorageIoResult::WriteFailed;
    }

    return StorageIoResult::Ok;
}
