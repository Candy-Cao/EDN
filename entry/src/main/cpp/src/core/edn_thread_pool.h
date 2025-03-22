//
// Created on 2025/3/22.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef EDN_EDNTHREADPOOL_H
#define EDN_EDNTHREADPOOL_H

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>

namespace edn {
class EdnThreadPool {
public:
     EdnThreadPool(size_t num_threads);
    ~EdnThreadPool();

    // 提交任务到线程池
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

private:
    // 工作线程
    std::vector<std::thread> workers;
    // 任务队列
    std::queue<std::function<void()>> tasks;

    // 同步原语
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
};
// 提交任务到线程池
template<class F, class... Args>
auto EdnThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // 不允许在停止线程池后添加任务
        if (stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }

        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}


typedef std::shared_ptr<EdnThreadPool> EdnThreadPoolPtr;

};


#endif //EDN_EDNTHREADPOOL_H
