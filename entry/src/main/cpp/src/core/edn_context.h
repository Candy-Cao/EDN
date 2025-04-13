//
// Created on 2025/3/16.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef EDN_EDN_CONTEXT_H
#define EDN_EDN_CONTEXT_H
#include "edn_config.h"
#include "edn_event.h"
#include "edn_event_listener.h"
#include "edn_thread_pool.h"
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>


namespace edn {

typedef std::list<EdnEventPtr>::iterator iterator;
class EdnContext {
public:
    EdnContext();
    
    ~EdnContext();
    
    //信号处理函数
    static void SigHandler(int sig);
    
    //获取信号管道read端
    int GetSigFd() {
        return pipe_fds_[0];
    }
    
    void AddEvent(EdnEventPtr event);
    
    void DelEvent(EdnEventPtr event);
    
    EdnEventListenerPtr GetListener() {
        return listener_;
    }
    EdnConfigPtr GetConfig() {
        return config_;
    }
    
    EdnThreadPoolPtr GetThreadPool() {
        return thread_pool_;
    }
    
    /**
     * 激活空闲事件
     */
    int ActiveEvent(int key, int events);
    
    /**
     * 事件分发主循环
     */
    void Run();
    
    /**
     * 结束事件循环
     */
    void Stop() {
        done_ = true;
    }
    
private:
    EdnEventListenerPtr listener_;
    EdnConfigPtr config_;
    static int pipe_fds_[2];
    std::list<EdnEventPtr> idle_queue_;
    std::unordered_map<int, iterator> table_;//对于IO事件，key是fd;对于信号事件，key为signal number;
    std::mutex idle_queue_mutex_;
//    std::list<EdnEventPtr> active_queue_;
//    std::mutex active_queue_mutex_;
    EdnThreadPoolPtr thread_pool_ = nullptr;
    std::atomic_bool done_;
};

typedef std::shared_ptr<EdnContext> EdnContextPtr;

}

#endif //EDN_EDN_CONTEXT_H
