#include "edn_timer.h"
#include "edn_log.h"

namespace edn {
    
EdnTimer::EdnTimer(int timeout, bool is_persist, std::function<void()> &&callback): EdnEvent()
{
    timeout_ = timeout;
    expire_time_ = EdnUtils::GetCurrentTime() + timeout_;
    if (is_persist) {
        events_ |= PERSIST;
    }
    events_ |= TIMEOUT;
    callback_ = std::move(callback); 
}

int EdnTimer::handler()
{

    EDN_LOG_INFO("timer event %d trigger.", eventId_);
    callback_();
    EdnEvent::handler();
    return EDN_OK;
}

} // namespace edn