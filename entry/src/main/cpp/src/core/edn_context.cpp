//
// Created on 2025/3/16.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "edn_context.h"
#include "edn_define.h"
#include "edn_epoll.h"
#include "edn_log.h"
#include "edn_utils.h"
#include "singleton.h"
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
    EdnUtils::SetNonBlocking(pipe_fds_[1]);
    listener_ = std::make_shared<EdnEpoll>(this);
    config_ = Singleton<EdnConfig>::getInstance();
    thread_pool_ = std::make_shared<EdnThreadPool>(config_->work_thread_num);
}

void EdnContext::SigHandler(int sig) {
    int save_errno = errno;
    
    int msg = sig;
    send(pipe_fds_[1], (char*)&msg, 1, 0);
    errno = save_errno;
}

void EdnContext::AddEvent(EdnEventPtr event) {
    {
        std::lock_guard<std::mutex> lock(idle_queue_mutex_);
        idle_queue_.push_back(event);
        int fd = event->GetFd();
        if (fd != INVALID_FD) {
            table_[fd] = (--(idle_queue_.end()));
        }
    }
    int ret = listener_->add(event);
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

int EdnContext::ActiveEvent(int key) {
    EdnEventPtr p = nullptr;
    {
        std::lock_guard<std::mutex> lock(idle_queue_mutex_);
        auto it = table_.find(key);
        if (it == table_.end()) {
            EDN_LOG_ERROR("event not find ,key:", key);
            return EDN_ERR_PARAMS_INVALID;
        }
        p = *it->second;
        p->Active();
    }
    EDN_LOG_INFO("event actived ,eventdId:%d, isPersist:%d, fd:%d", p->GetUUID(), p->IsPersist(), p->GetFd());
    if (!p->IsPersist()) {
        DelEvent(p);
    }
    return EDN_OK;
}

void EdnContext::Run() {
    while (!done_) {
        int timeout = 1000;
        int ret = listener_->dispatch(timeout);
        if (ret < 0) {
            EDN_LOG_ERROR("epoll wait failed, errno:%d", errno);
            return;
        }
    }
}

}