#include <unity.h>

#include "modules/storage/storage_io.h"

struct FakeIoCtx {
    bool fileExists = false;
    bool removeOk = true;
    bool openOk = true;
    bool writeOk = true;
    bool openAppend = false;
    bool closeCalled = false;
    int removeCalls = 0;
    int openCalls = 0;
    int writeCalls = 0;
};

static bool fakeExists(void* ctx, const char*) {
    return static_cast<FakeIoCtx*>(ctx)->fileExists;
}

static bool fakeRemove(void* ctx, const char*) {
    FakeIoCtx* c = static_cast<FakeIoCtx*>(ctx);
    c->removeCalls++;
    if (!c->removeOk) return false;
    c->fileExists = false;
    return true;
}

static bool fakeOpenWrite(void* ctx, const char*, bool append) {
    FakeIoCtx* c = static_cast<FakeIoCtx*>(ctx);
    c->openCalls++;
    c->openAppend = append;
    return c->openOk;
}

static bool fakeWriteText(void* ctx, const char*) {
    FakeIoCtx* c = static_cast<FakeIoCtx*>(ctx);
    c->writeCalls++;
    return c->writeOk;
}

static void fakeClose(void* ctx) {
    static_cast<FakeIoCtx*>(ctx)->closeCalled = true;
}

static StorageIo makeIo(FakeIoCtx* ctx) {
    StorageIo io = { fakeExists, fakeRemove, fakeOpenWrite, fakeWriteText, fakeClose, ctx };
    return io;
}

void test_overwrite_existing_truncates_before_write() {
    FakeIoCtx ctx;
    ctx.fileExists = true;
    StorageIo io = makeIo(&ctx);

    StorageIoResult result = storageWriteText(&io, "/file.txt", "abc", StorageWriteMode::Overwrite);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(StorageIoResult::Ok), static_cast<int>(result));
    TEST_ASSERT_EQUAL(1, ctx.removeCalls);
    TEST_ASSERT_EQUAL(1, ctx.openCalls);
    TEST_ASSERT_FALSE(ctx.openAppend);
    TEST_ASSERT_EQUAL(1, ctx.writeCalls);
    TEST_ASSERT_TRUE(ctx.closeCalled);
}

void test_append_does_not_truncate() {
    FakeIoCtx ctx;
    ctx.fileExists = true;
    StorageIo io = makeIo(&ctx);

    StorageIoResult result = storageWriteText(&io, "/file.txt", "abc", StorageWriteMode::Append);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(StorageIoResult::Ok), static_cast<int>(result));
    TEST_ASSERT_EQUAL(0, ctx.removeCalls);
    TEST_ASSERT_EQUAL(1, ctx.openCalls);
    TEST_ASSERT_TRUE(ctx.openAppend);
}

void test_overwrite_fails_if_truncate_failed() {
    FakeIoCtx ctx;
    ctx.fileExists = true;
    ctx.removeOk = false;
    StorageIo io = makeIo(&ctx);

    StorageIoResult result = storageWriteText(&io, "/file.txt", "abc", StorageWriteMode::Overwrite);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(StorageIoResult::TruncateFailed), static_cast<int>(result));
    TEST_ASSERT_EQUAL(0, ctx.openCalls);
}

void test_write_fails_if_open_failed() {
    FakeIoCtx ctx;
    ctx.openOk = false;
    StorageIo io = makeIo(&ctx);

    StorageIoResult result = storageWriteText(&io, "/file.txt", "abc", StorageWriteMode::Overwrite);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(StorageIoResult::OpenFailed), static_cast<int>(result));
    TEST_ASSERT_FALSE(ctx.closeCalled);
}

void test_write_fails_if_writer_failed() {
    FakeIoCtx ctx;
    ctx.writeOk = false;
    StorageIo io = makeIo(&ctx);

    StorageIoResult result = storageWriteText(&io, "/file.txt", "abc", StorageWriteMode::Overwrite);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(StorageIoResult::WriteFailed), static_cast<int>(result));
    TEST_ASSERT_TRUE(ctx.closeCalled);
}

void test_invalid_args_rejected() {
    FakeIoCtx ctx;
    StorageIo io = makeIo(&ctx);

    StorageIoResult result = storageWriteText(&io, "", "abc", StorageWriteMode::Overwrite);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(StorageIoResult::InvalidArgs), static_cast<int>(result));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_overwrite_existing_truncates_before_write);
    RUN_TEST(test_append_does_not_truncate);
    RUN_TEST(test_overwrite_fails_if_truncate_failed);
    RUN_TEST(test_write_fails_if_open_failed);
    RUN_TEST(test_write_fails_if_writer_failed);
    RUN_TEST(test_invalid_args_rejected);
    return UNITY_END();
}
