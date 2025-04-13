//
// Created on 2025/3/14.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef EDN_EDNEVENT_H
#define EDN_EDNEVENT_H


#include "edn_define.h"
#include "edn_utils.h"
#include "edn.h"
#include <memory>

namespace edn {
class EdnContext;

typedef enum {
    WRITE = 1,
    READ = 2,
    PERSIST = 4,
    SIGNAL = 8,
    TIMEOUT = 16,
    ERROR = 32,
    CLOSE = 64,
    CONNECT = 128,
    ET = 256,
} EdnEventType;

typedef enum {
    IDLE,
    ACTIVE,
} EdnEventStatus;

class EdnEvent: public std::enable_shared_from_this<EdnEvent> {
public:
    EdnEvent();
    EdnEvent(int fd, unsigned int events);
    ~EdnEvent() = default;
    
    virtual int handler() {status_ = IDLE; return EDN_OK;};
    void Active();
    virtual int GetFd() {
        return fd_;
    }
    unsigned int GetEvents();
    std::shared_ptr<EdnContext> GetContext() {
        return context_.lock();
    }
    
    bool IsPersist() {
        return events_ & PERSIST;
    }
    
    int GetUUID() {
        return eventId_;
    }
protected:
    int fd_;
    unsigned int events_;
    std::weak_ptr<EdnContext> context_; 
    unsigned int eventId_ = EdnUtils::GetUUID();
    int status_ = IDLE;  
};

typedef std::shared_ptr<EdnEvent> EdnEventPtr;

}
#endif //EDN_EDNEVENT_H
