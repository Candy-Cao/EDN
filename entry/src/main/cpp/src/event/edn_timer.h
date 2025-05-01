/**
 * @file edn_timer.h
 * @brief 定时器事件
 * @author candy
 * @date 2025/3/16
 * @version 1.0         
 */

#ifndef EDN_EDN_TIMER_H
#define EDN_EDN_TIMER_H
#include "edn_event.h"
#include <chrono>
#include <functional>

namespace edn {
class EdnTimer: public EdnEvent {
public:
    EdnTimer(int timeout, bool is_persist, std::function<void()> &&callback = nullptr);
    
    
    int handler() override ;
    
    int GetTimeout() {
        return timeout_;
    }
    void SetTimeout(int timeout) {
        timeout_ = timeout;
    }
    void SetCallback(std::function<void()> callback) {
        callback_ = callback;
    }
    std::function<void()> GetCallback() {
        return callback_;
    }
    int GetExpireTime() {
        return expire_time_;
    }
    void SetExpireTime(int expire_time) {
        expire_time_ = expire_time;
    }
    bool IsEnabled() {
        return enabled_;
    }
    void SetEnabled(bool enabled) {
        enabled_ = enabled;
    }
private:
    int timeout_ = 0;
    int expire_time_ = 0;
    bool enabled_ = true;
    std::function<void()> callback_;
    
};

typedef std::shared_ptr<EdnTimer> EdnTimerPtr;
}   // namespace edn


#endif //EDN_EDN_TIMER_H