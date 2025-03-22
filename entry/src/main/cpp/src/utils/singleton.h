//
// Created on 2025/3/16.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef EDN_SINGLETON_H
#define EDN_SINGLETON_H
#include <memory>
#include <utility> 
namespace edn {


// 单例模式类模板
template <typename T>
class Singleton {
public:
    // 删除拷贝构造函数和赋值运算符
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    // 获取单例实例的静态方法
    template <typename... Args>
    static std::shared_ptr<T>& getInstance(Args&&... args) {
        static std::shared_ptr<T> instance = std::make_shared<T>(std::forward<Args>(args)...); // C++11 保证静态局部变量的线程安全性
        return instance;
    }

protected:
    // 构造函数和析构函数为 protected，防止外部直接实例化
    Singleton() = default;
    ~Singleton() = default;
};
}
#endif //EDN_SINGLETON_H
