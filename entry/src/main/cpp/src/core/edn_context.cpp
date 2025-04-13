//
// Created on 2025/3/16.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "edn_context.h"
#include "edn_define.h"
#include "edn_epoll.h"
#include "edn_log.h"
#include "edn_signal.h"
#include "edn_utils.h"
#include "edn_io_event.h"
#include "singleton.h"
#include "edn_socket_utils.h"
#include <cassert>
#include <sys/socket.h>
#include <errno.h>

extern int errno;

namespace edn {

int EdnContext::pipe_fds_[2] = {0};

EdnContext::EdnContext() {
    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipe_fds_);
    assert(ret != -1);
    EDN_LOG_INFO("create signal pipe success. write fd: %d, read fd: %d", pipe_fds_[1], pipe_fds_[0]);
    SetNonBlocking(pipe_fds_[1]);
    SetNonBlocking(pipe_fds_[0]);
    listener_ = std::make_shared<EdnEpoll>(this);
    config_ = Singleton<EdnConfig>::getInstance();
    thread_pool_ = std::make_shared<EdnThreadPool>(config_->work_thread_num);
}

EdnContext::~EdnContext() {
    close(pipe_fds_[0]);
    close(pipe_fds_[1]);
}

void EdnContext::SigHandler(int sig) {
    int save_errno = errno;
    int msg = sig;
    send(pipe_fds_[1], (char*)&msg, 1, 0);
    errno = save_errno;
    EDN_LOG_INFO( "I caugh the signal %d\n", sig );
}

void EdnContext::AddEvent(EdnEventPtr event) {
    {
        std::lock_guard<std::mutex> lock(idle_queue_mutex_);
        int fd = event->GetFd();
        if (fd != INVALID_FD) {//非定时器事件才需要入队
            idle_queue_.push_back(event);
            table_[fd] = (--(idle_queue_.end()));
        }
    }
    int ret = listener_->add(event);
    EDN_LOG_INFO("add event, eventId:%d, fd:%d", event->GetUUID(), event->GetFd());
    assert(ret == 0);
}

void EdnContext::DelEvent(EdnEventPtr event) {
    {
        std::lock_guard<std::mutex> lock(idle_queue_mutex_);
        int fd = event->GetFd();
        if (fd != INVALID_FD) {
            auto it = table_[fd];
            idle_queue_.erase(it);
            table_.erase(fd);
        }
    }
    int ret = listener_->del(event);
    assert(ret == 0);
}

int EdnContext::ActiveEvent(int key, int events) {
    EdnEventPtr p = nullptr;
    {
        std::lock_guard<std::mutex> lock(idle_queue_mutex_);
        auto it = table_.find(key);
        if (it == table_.end()) {
            EDN_LOG_ERROR("event not find ,key:", key);
            return EDN_ERR_PARAMS_INVALID;
        }
        p = *it->second;
        if (p->GetFd() != GetSigFd()) {//IO事件激活
            auto io_event = std::dynamic_pointer_cast<EdnIOEvent>(p);
            events = io_event->ConvertEvents(events);
            io_event->SetRealEvents(events);
        }
        p->Active();
    }
    EDN_LOG_INFO("event actived ,eventdId:%d, isPersist:%d, fd:%d, events:%d", p->GetUUID(), p->IsPersist(), p->GetFd(), events);
    if (!p->IsPersist()) {
        DelEvent(p);
    }
    return EDN_OK;
}

void EdnContext::Run() {
    int timeout = listener_->GetTimeout();
    while (!done_) {
        if (timeout < 0 && idle_queue_.empty()) {
            EDN_LOG_WARN("no event to dispatch, exiting...");
            sleep(1);// sleep 1s ,wait for active event handler;
            return ;
        }
        int ret = listener_->dispatch(&timeout);
        if (ret < 0) {
            EDN_LOG_ERROR("epoll wait failed, errno:%d", errno);
            return;
        }
        EDN_LOG_INFO("a round of event dispatch end... timeout:%d", timeout);
    }
    EDN_LOG_INFO("event loop exit.");
}

}