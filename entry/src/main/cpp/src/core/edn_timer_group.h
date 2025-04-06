/**
 * 
 * @file edn_timer_group.h
 * @brief 定时器组
 * @author candy
 * @date 2025/3/16
 * @version 1.0
 * 
 */

#ifndef EDN_EDN_TIMER_GROUP_H
#define EDN_EDN_TIMER_GROUP_H
#include "edn_event.h"
#include "edn_timer.h"
#include <queue>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <functional>


namespace edn {
class EdnTimerGroup {
public:
~EdnTimerGroup() = default;
EdnTimerGroup() = default;
    
    int AddTimer(EdnTimerPtr timer);
    
    int DelTimer(EdnTimerPtr timer);
    
    int Dispatch();

    int GetMinTimeout();

private:
    std::priority_queue<EdnTimerPtr, std::vector<EdnTimerPtr>, std::function<bool(EdnTimerPtr, EdnTimerPtr)>> timer_queue_{
        [](EdnTimerPtr a, EdnTimerPtr b) {
            return a->GetExpireTime() > b->GetExpireTime();
        }
    };
    std::mutex timer_mutex_;
    std::unordered_map<int, EdnTimerPtr> timer_map_;
    EdnContext *context_;
};
typedef std::shared_ptr<EdnTimerGroup> EdnTimerGroupPtr;
}

#endif //EDN_EDN_TIMER_GROUP_H