//
// Created on 2025/3/14.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".
#include "edn_context.h"
#include "edn_event.h"
#include "edn_context.h"
#include "singleton.h"

namespace edn {
EdnEvent::EdnEvent() : fd_(INVALID_FD), events_(0) {
    context_ = Singleton<EdnContext>::getInstance();
}

EdnEvent::EdnEvent(int fd, unsigned int events): fd_(fd), events_(events) {
    context_ = Singleton<EdnContext>::getInstance();
}

unsigned int EdnEvent::GetEvents() {
    return GetContext()->GetListener()->convert_events(events_);
}

void EdnEvent::Active() {
    status_ = ACTIVE;
    context_.lock()->GetThreadPool()->enqueue(std::bind(&EdnEvent::handler, this));
}

}
