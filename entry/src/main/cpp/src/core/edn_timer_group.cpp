#include "edn_timer_group.h"
#include "edn_log.h"

namespace edn
{
    



int EdnTimerGroup::AddTimer(EdnTimerPtr timer)
{
    std::lock_guard<std::mutex> lock(timer_mutex_);
    timer_queue_.push(timer);
    timer_map_[timer->GetUUID()] = timer;
    EDN_LOG_INFO("add timer, eventId:%d, timeout:%d", timer->GetUUID(), timer->GetTimeout());\
    return EDN_OK;
}

int EdnTimerGroup::DelTimer(EdnTimerPtr timer)
{
    std::lock_guard<std::mutex> lock(timer_mutex_);
    auto it = timer_map_.find(timer->GetUUID());
    if (it != timer_map_.end()) {
        if (it->second == timer_queue_.top()) {
            timer_queue_.pop();
        }
        else {
           it->second->SetEnabled(false); 
        }
        EDN_LOG_INFO("delete timer, eventId:%d", timer->GetUUID());
        timer_map_.erase(it);
        return EDN_OK;
    }
    return EDN_ERR_PARAMS_INVALID;
}

int EdnTimerGroup::Dispatch()
{
    std::lock_guard<std::mutex> lock(timer_mutex_);
    while (!timer_queue_.empty()) {
        auto timer = timer_queue_.top();
        if (!timer->IsEnabled()) {
            timer_queue_.pop();
            continue;
        }
        int current_time = EdnUtils::GetCurrentTime();
        if (current_time >= timer->GetExpireTime()) {
            timer->Active();
            timer_queue_.pop();
            EDN_LOG_INFO("dispatch timer event, eventId:%d", timer->GetUUID());
            if (timer->IsPersist()) {
                timer->SetExpireTime(current_time + timer->GetTimeout());
                timer_queue_.push(timer);
            }
            else {
                timer_map_.erase(timer->GetUUID());
            }
        }
        else {
            break;
        }
      
    }
    if (timer_queue_.empty()) {
        return -1;
    }
    return timer_queue_.top()->GetExpireTime() - EdnUtils::GetCurrentTime();
}

int EdnTimerGroup::GetMinTimeout()
{
    std::lock_guard<std::mutex> lock(timer_mutex_);
    if (timer_queue_.empty()) {
        return -1;
    }
    return timer_queue_.top()->GetExpireTime() - EdnUtils::GetCurrentTime();
}

} // namespace EDN