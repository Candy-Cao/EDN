//
// Created on 2025/3/16.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef EDN_EDN_EPOLL_H
#define EDN_EDN_EPOLL_H
#include "edn_event_listener.h"
#include <cstdint>
#include <sys/epoll.h>

namespace edn {
class EdnEpoll:public EdnEventListener {
public:
    EdnEpoll(EdnContext *ctx);
    ~EdnEpoll();
    
    int add(EdnEventPtr event) override ;
    int del(EdnEventPtr event) override ;
    int dispatch(int timeout) override ;
    
    uint32_t convert_events(int events) override ;
    EdnContext* GetContext() {
        return context_;
    }
    int DispatchSignal();
private:
    int epfd_;
    EdnContext *context_;
    epoll_event *events_;
};
}

#endif //EDN_EDN_EPOLL_H
