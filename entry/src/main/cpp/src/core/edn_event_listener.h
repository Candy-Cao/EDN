//
// Created on 2025/3/14.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef EDN_EDN_EVENT_LISTENER_H
#define EDN_EDN_EVENT_LISTENER_H

#include "edn_event.h"
#include <memory>
#include <cstdint>
#include <sys/time.h>


namespace edn {
class EdnEventListener {
public:
    EdnEventListener() = default;
    
    virtual int add(EdnEventPtr event) = 0;
    virtual int del(EdnEventPtr event) = 0;
    virtual int dispatch(int timeout) = 0;
    
    virtual uint32_t convert_events(int events) = 0;
};

typedef std::shared_ptr<EdnEventListener> EdnEventListenerPtr;
}

#endif //EDN_EDN_EVENT_LISTENER_H
