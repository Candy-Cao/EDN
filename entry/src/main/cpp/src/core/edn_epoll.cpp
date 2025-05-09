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
    epfd_ = epoll_create(Singleton<EdnConfig>::getInstance()->max_event_num);
    assert(epfd_ > 0);
    EDN_LOG_INFO("create epoll success: epfd:%d", epfd_);
    events_ = new epoll_event[Singleton<EdnConfig>::getInstance()->max_event_num];
    memset(events_, 0, sizeof(epoll_event) * Singleton<EdnConfig>::getInstance()->max_event_num);
    if (!events_) {
        EDN_LOG_ERROR("memory alloc failed. ");
    }
    int sigfd = context_->GetSigFd();
    struct epoll_event ev;
    ev.data.fd = sigfd;
    ev.events = EPOLLIN;
    int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, sigfd, &ev);
    if (ret == -1) {
        EDN_LOG_ERROR("system call epoll_ctl failed. errno:%d", errno);
        return;
    }
    timer_group_ = std::make_shared<EdnTimerGroup>();
}

EdnEpoll::~EdnEpoll() {
    delete[] events_;
    events_ = nullptr;
}

int EdnEpoll::add(EdnEventPtr event) {
    if (event->GetFd() == INVALID_FD) {//定时器事件
        auto timer = std::dynamic_pointer_cast<EdnTimer>(event);
        if (timer->GetExpireTime() <= EdnUtils::GetCurrentTime()) {
            context_->GetThreadPool()->enqueue(std::bind(&EdnTimer::handler, timer));
            return EDN_OK;
        }
        timer_group_->AddTimer(timer);
        return EDN_OK;
    }
    epoll_event ev;
    ev.data.fd = event->GetFd();
    if (event->GetFd() < 0) {//信号事件
        struct sigaction sa, old_sa;
        memset(&sa, '\0', sizeof(sa));
        memset(&old_sa, '\0', sizeof(sa));
        sa.sa_handler = EdnContext::SigHandler;
        sa.sa_flags |= SA_RESTART; 
        sigfillset(&sa.sa_mask);
        int sig = std::dynamic_pointer_cast<EdnSignal>(event)->GetSignal();
        sigaction(sig, &sa, &old_sa);
        EDN_LOG_INFO("add signal %d to epoll.", sig);
        std::dynamic_pointer_cast<EdnSignal>(event)->SetOldSigaction(&old_sa);
        return EDN_OK;
    }
    ev.events = event->GetEvents();
    EDN_LOG_INFO("add listen for eventId:%d, fd:%d with event write:%d and read:%d to epfd:%d", event->GetUUID(), ev.data.fd, ev.events & EPOLLOUT, ev.events & EPOLLIN, epfd_);
    int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, ev.data.fd, &ev);
    if (ret == -1 && errno == EEXIST) {
        EDN_LOG_WARN("fd:%d already exist in epoll, modify it.", ev.data.fd);
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
        auto timer = std::dynamic_pointer_cast<EdnTimer>(event);
        timer_group_->DelTimer(timer);
        return EDN_OK;
    }
    if (event->GetFd() < 0) {
        auto sig_event = std::dynamic_pointer_cast<EdnSignal>(event);
        sigaction(sig_event->GetSignal(), sig_event->GetOldSigaction(), NULL);
        EDN_LOG_INFO("delete signal %d from epoll.", sig_event->GetSignal());
        return EDN_OK;
    }
    int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, event->GetFd(), NULL);
    if (ret == -1 && errno != EBADF) {
        EDN_LOG_ERROR("syscall epoll_ctl failed, ret:%d, errno:%d", ret, errno);
        return EDN_ERR_SYS_ERROR;
    }
    return EDN_OK;
}

int EdnEpoll::dispatch(int *timeout) {
    int max_event_num = GetContext()->GetConfig()->max_event_num;
    EDN_LOG_INFO("epoll wait begin, timeout:%d", *timeout);
    int num = epoll_wait(epfd_, events_, max_event_num, *timeout);
    if (num == -1 && errno != EINTR) {
        EDN_LOG_ERROR("call epoll_wait error: %d", errno);
        return errno;
    }
    for (int i = 0; i < num; i++) {
        int fd = events_[i].data.fd;
        if (fd == context_->GetSigFd()) {//处理信号事件
            DispatchSignal();
            continue;
        }
        //TODO:IO event dispatch
        auto ret = context_->ActiveEvent(fd, events_[i].events);
        if (ret != EDN_OK) {
            EDN_LOG_ERROR("active event error: %d", ret);
            return ret;
        }
    }
    *timeout = timer_group_->Dispatch();
    return EDN_OK;
}

uint32_t EdnEpoll::convert_events(int events) {
    int ev = 0;
    if (events & WRITE) {
        ev |= (EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP);//EPOLLONESHOT：触发事件后需要重新注册到epoll上
    }
    if (events & READ) {
        ev |= (EPOLLIN | EPOLLONESHOT | EPOLLRDHUP);
    }
    if (events & SIGNAL) {
        ev |= EPOLLIN;
    }
    if (events & ET) {
        ev |= EPOLLET;
    }
    return ev;
}

int EdnEpoll::DispatchSignal() {
    char signals[1];//信号触发并不频繁，每次主循环激活一个信号事件；
    int num = read(context_->GetSigFd(), signals, sizeof(signals));
    if (num == -1) {
        EDN_LOG_ERROR("read signals error: %d", errno);
        return errno;
    }
    context_->ActiveEvent(HASH(signals[0]), 0);
    return EDN_OK;
}

}