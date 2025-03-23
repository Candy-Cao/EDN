//
// Created on 2025/3/22.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "edn_thread_pool.h"
#include <signal.h>

namespace edn {

// 构造函数，启动工作线程
EdnThreadPool::EdnThreadPool(size_t num_threads) : stop(false) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                    if (this->stop && this->tasks.empty()) {
                        return;
                    }
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }

                task();
            }
        });
    }
}

// 析构函数，停止所有线程
EdnThreadPool::~EdnThreadPool() {
    stop = true;
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}


}