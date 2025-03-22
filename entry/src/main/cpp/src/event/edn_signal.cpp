//
// Created on 2025/3/16.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "edn_signal.h"
#include "edn_log.h"
#include "edn_context.h"
#include <cassert>

namespace edn {
EdnSignal::EdnSignal(int sig, bool is_persist, std::function<void()> callback): EdnEvent(), sig_(sig), callback_(callback) {
    fd_ = context_.lock()->GetSigFd();
    if (is_persist) {
        events_ |= PERSIST;
    }
    events_ |= SIGNAL;
}

int EdnSignal::handler() {
    EDN_LOG_INFO("signale %d trigger %d times total.", sig_, ++trigger_cnt_);
    callback_();
    EdnEvent::handler();
} 

}