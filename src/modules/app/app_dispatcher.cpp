#include "app_dispatcher.h"

static constexpr size_t APP_EVENT_QUEUE_SIZE = 16;
static AppEvent queueBuffer[APP_EVENT_QUEUE_SIZE];
static size_t queueHead = 0;
static size_t queueTail = 0;
static size_t droppedEvents = 0;

static size_t nextIndex(size_t index) {
    return (index + 1) % APP_EVENT_QUEUE_SIZE;
}

void appDispatcherInit() {
    queueHead = 0;
    queueTail = 0;
    droppedEvents = 0;
}

bool appDispatcherPost(AppEventType type) {
    const size_t newTail = nextIndex(queueTail);
    if (newTail == queueHead) {
        droppedEvents++;
        return false;
    }

    AppEvent event;
    event.type = type;
    event.timestampMs = millis();
    queueBuffer[queueTail] = event;
    queueTail = newTail;
    return true;
}

bool appDispatcherNext(AppEvent& out) {
    if (queueHead == queueTail) {
        return false;
    }

    out = queueBuffer[queueHead];
    queueHead = nextIndex(queueHead);
    return true;
}

size_t appDispatcherDroppedCount() {
    return droppedEvents;
}
