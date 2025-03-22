//
// Created on 2025/3/16.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "edn_epoll.h"
#include "edn.h"
#include "edn_define.h"
#include "edn_context.h"
#include "edn_log.h"
#include "edn_signal.h"
#include "singleton.h"
#include <cerrno>
#include <cstring>
#include <cstddef>
#include <sys/epoll.h>
#include <errno.h>
#include <signal.h>
#include <cassert>
#include <fcntl.h>

extern int errno;

namespace edn {
EdnEpoll::EdnEpoll(EdnContext *ctx): context_(ctx) {
    epfd_ = epoll_create(0);
    EDN_LOG_INFO("create epoll success: epfd:%d", epfd_);
    events = new epoll_event[Singleton<EdnConfig>::getInstance()->max_event_num];
    if (!events) {
        EDN_LOG_ERROR("memory alloc failed. ");
    }
}

EdnEpoll::~EdnEpoll() {
    delete[] events;
    events = nullptr;
}

int EdnEpoll::add(EdnEventPtr event) {
    if (event->GetFd() == INVALID_FD) {//定时器事件
        return EDN_OK;
    }
    epoll_event ev;
    ev.data.fd = event->GetFd();
    if (event->GetFd() < 0) {//信号事件
        ev.data.fd = event->GetContext()->GetSigFd();
        struct sigaction sa, old_sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = EdnContext::SigHandler;
        sa.sa_flags |= SA_RESTART;
        sigfillset(&sa.sa_mask);
        int sig = std::dynamic_pointer_cast<EdnSignal>(event)->GetSignal();
        assert(sigaction(sig, &sa, &old_sa) != -1);
        std::dynamic_pointer_cast<EdnSignal>(event)->SetOldSigaction(&old_sa);
    }
    ev.events = event->GetEvents();
    EDN_LOG_INFO("add listen for fd:%d with event write:%d and read:%d to epfd:%d", ev.data.fd, ev.events & EPOLLOUT, ev.events & EPOLLIN, epfd_);
    int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, ev.data.fd, &ev);
    if (ret == EEXIST) {
        ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, ev.data.fd, &ev);
    }
    if (ret == -1) {
        EDN_LOG_ERROR("system call epoll_ctl failed. errno:%d", errno);
        return EDN_ERR_SYS_ERROR;
    }
    return EDN_OK;
}

int EdnEpoll::del(EdnEventPtr event) {
    EDN_LOG_INFO("delete event, eventId:%d, fd:%d", event->GetUUID(), event->GetFd());
    if (event->GetFd() == INVALID_FD) {//定时器事件
        return EDN_OK;
    }
    if (event->GetFd() < 0) {
        auto sig_event = std::dynamic_pointer_cast<EdnSignal>(event);
        sigaction(sig_event->GetSignal(), sig_event->GetOldSigaction(), NULL);
        return EDN_OK;
    }
    int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, event->GetFd(), NULL);
    if (ret == -1) {
        return EDN_ERR_SYS_ERROR;
    }
    return EDN_OK;
}

int EdnEpoll::dispatch(int timeout) {
    int max_event_num = GetContext()->GetConfig()->max_event_num;
    int num = epoll_wait(epfd_, events, max_event_num, timeout);
    if (num == -1 && errno != EINTR) {
        EDN_LOG_ERROR("call epoll_wait error: %d", errno);
        return errno;
    }
    for (int i = 0; i < num; i++) {
        int fd = events[i].data.fd;
        if (fd == context_->GetSigFd()) {//处理信号事件
            DispatchSignal();
            continue;
        }
        //TODO:IO event dispatch
    }
    return EDN_OK;
}

uint32_t EdnEpoll::convert_events(int events) {
    int ev = 0;
    if (events & WRITE) {
        ev |= (EPOLLOUT | EPOLLET | EPOLLONESHOT);
    }
    if (events & READ) {
        ev |= (EPOLLIN | EPOLLET | EPOLLONESHOT);
    }
    if (events & SIGNAL) {
        ev |= EPOLLIN;
    }
    return ev;
}

int EdnEpoll::DispatchSignal() {
    char signals[1];//信号触发并不平凡，每次主循环激活一个信号事件；
    int num = read(context_->GetSigFd(), signals, sizeof(signals));
    if (num == -1) {
        EDN_LOG_ERROR("read signals error: %d", errno);
        return errno;
    }
    context_->ActiveEvent(HASH(signals[0]));
    return EDN_OK;
}

}