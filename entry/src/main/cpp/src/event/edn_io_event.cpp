#include "edn_io_event.h"
#include "edn_log.h"
#include "edn_context.h"
#include "edn_socket_utils.h"
#include <errno.h>
#include <sys/epoll.h>

namespace edn {

EdnIOEventCallback EdnIOEvent::def_conn_cb = [](void* p) {\
    EDN_LOG_INFO("default conn cb");
    return EDN_OK;
};
EdnIOEventCallback EdnIOEvent::def_read_cb = [](void* p) {
    auto now = EdnUtils::GetCurrentTime();
    static int last_time = EdnUtils::GetCurrentTime();
    if (now - last_time < 1000) {
        return EDN_OK;
    }
    EDN_LOG_INFO("default read cb");
    return EDN_OK;
};
EdnIOEventCallback EdnIOEvent::def_write_cb = [](void* p) {
    auto now = EdnUtils::GetCurrentTime();
    static int last_time = EdnUtils::GetCurrentTime();
    if (now - last_time < 1000) {
        return EDN_OK;
    }
    EDN_LOG_INFO("default write cb");
    return EDN_OK;
};
EdnIOEventCallback EdnIOEvent::def_error_cb = [](void* p) {
    EDN_LOG_INFO("default error cb");
    auto that = (EdnIOEvent*)p;
    int error = 0;
    socklen_t len = sizeof(error);
    if (getsockopt(that->fd_, SOL_SOCKET, SO_ERROR, &error, &len) == -1) {
        EDN_LOG_ERROR("getsockopt error: %d", errno);
        return EDN_ERR_SYS_ERROR;
    }
    EDN_LOG_ERROR("socket error: %d", error);
    return EDN_OK;
};
EdnIOEventCallback EdnIOEvent::def_close_cb = [](void* p) {
    EDN_LOG_INFO("default close cb");
    return EDN_OK;
};
EdnIOEvent::EdnIOEvent(int fd, EdnIOEventCallback conn_cb,
                       EdnIOEventCallback read_cb,
                       EdnIOEventCallback write_cb,
                       EdnIOEventCallback error_cb,
                       EdnIOEventCallback close_cb,
                       bool et)
    : EdnEvent(fd, EdnEventType::READ | EdnEventType::WRITE | EdnEventType::PERSIST | EdnEventType::CONNECT | EdnEventType::CLOSE | EdnEventType::ERROR),
      conn_cb_(conn_cb),
      read_cb_(read_cb),
      write_cb_(write_cb),
      error_cb_(error_cb),
      close_cb_(close_cb),
      connect_status_(ConnectStatus::CONNECTING) 
{
    if (et) {
        events_ |= EdnEventType::ET;
    }
}

EdnIOEvent::~EdnIOEvent()
{
    EDN_LOG_INFO("IO event destroy, fd:%d", fd_);
    if (fd_ != INVALID_FD) {
        Close(fd_);
        fd_ = INVALID_FD;
    }
}

int EdnIOEvent::handler()
{
    if (real_events_ & EdnEventType::READ) {
        context_.lock()->GetThreadPool()->enqueue([&](void* p) {
            read_cb_(this);
            auto ret = context_.lock()->GetListener()->add(shared_from_this());//重新注册事件
            if (ret != EDN_OK) {
                EDN_LOG_ERROR("read_cb_ end, add event error: %d", ret);
            }
        }, this);
    }
    if (real_events_ & EdnEventType::WRITE) {
        context_.lock()->GetThreadPool()->enqueue([&](void* p) {
            write_cb_(this);
            auto ret = context_.lock()->GetListener()->add(shared_from_this());//重新注册事件
            if (ret != EDN_OK) {
                EDN_LOG_ERROR("write_cb_ end, add event error: %d", ret);
            }
        }, this);
    }
    if (real_events_ & EdnEventType::CONNECT) {
        context_.lock()->GetThreadPool()->enqueue([&](void* p) {
            conn_cb_(this);
            auto ret = context_.lock()->GetListener()->add(shared_from_this());//重新注册事件
            if (ret != EDN_OK) {
                EDN_LOG_ERROR("conn_cb_ end, add event error: %d", ret);
            }
        }, this);
    }
    if (real_events_ & EdnEventType::CLOSE) {
        context_.lock()->GetThreadPool()->enqueue(close_cb_, this);
    }
    if (real_events_ & EdnEventType::ERROR) {
        context_.lock()->GetThreadPool()->enqueue(error_cb_, this);
    }
    real_events_ = 0;
    return EdnEvent::handler();
}

void EdnIOEvent::SetRealEvents(unsigned int events)
{
    real_events_ |= events;
}

int EdnIOEvent::ConvertEvents(unsigned int events)
{
    unsigned int ev = 0;
    if (events & EPOLLOUT && connect_status_ == ConnectStatus::CONNECTING) {
        ev |= EdnEventType::CONNECT;
        connect_status_ = ConnectStatus::CONNECTED;
    }
    if (events & EPOLLOUT) {
        ev |=  EdnEventType::WRITE;
        connect_status_ = ConnectStatus::WRITE;
    }
    if (events & EPOLLIN) {
        ev |= EdnEventType::READ;
        connect_status_ = ConnectStatus::READ;
    }
    if (events & (EPOLLHUP | EPOLLRDHUP)) {
        ev |= EdnEventType::CLOSE;
        connect_status_ = ConnectStatus::CLOSE;
    }
    if (events & EPOLLERR) {
        ev |= EdnEventType::ERROR;
        connect_status_ = ConnectStatus::ERROR;
    }
    
    return ev;
}

EdnIOEvent::EdnIOEvent(int fd, bool et): EdnEvent(fd, EdnEventType::READ | EdnEventType::WRITE | EdnEventType::PERSIST | EdnEventType::CONNECT | EdnEventType::CLOSE | EdnEventType::ERROR), 
                                        connect_status_(ConnectStatus::CONNECTING) {
    conn_cb_ = def_conn_cb;
    read_cb_ = def_read_cb;
    write_cb_ = def_write_cb;
    error_cb_ = def_error_cb;
    close_cb_ = def_close_cb;
    if (et) {
        events_ |= EdnEventType::ET;
    }
}


}