//
// Created on 2025/3/16.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef EDN_EDN_SIGNAL_H
#define EDN_EDN_SIGNAL_H

#include "edn_define.h"
#include "edn_event.h"
#include <csignal>
#include <cstring>
#include <functional>
#include <signal.h>
namespace edn {
#define HASH(x) (-((x) + 1))

class EdnSignal: public EdnEvent {
public:
    EdnSignal() = default;
    EdnSignal(int sig, bool is_persist, std::function<void()> cb);
    
    int GetFd() {
        return HASH(sig_);//防止sig和fd的值一样；防止值为-1；
    }
    
    int GetSignal() {
        return sig_;
    }
    
    void SetOldSigaction(struct sigaction *sa) {
        memcpy(&old_sa, sa, sizeof(old_sa));
    }
    
    struct sigaction* GetOldSigaction() {
        return &old_sa;
    }
    
    int handler() override ;
private:
    int sig_ = INVALID_SIG;
    std::function<void()> callback_;
    struct sigaction old_sa;
    int trigger_cnt_ = 0;
};
}

#endif //EDN_EDN_SIGNAL_H
    