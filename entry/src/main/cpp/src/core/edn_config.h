//
// Created on 2025/3/16.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef EDN_EDN_CONFIG_H
#define EDN_EDN_CONFIG_H

#include <memory>
namespace edn {
#define DEFAULT_MAX_EVENT_NUM 1024
#define DEFAULTWORK_THREAD_NUM 8

class EdnConfig {
public:
    EdnConfig() = default;

    unsigned int switchs_{0};
    int max_event_num = DEFAULT_MAX_EVENT_NUM;
    int work_thread_num = DEFAULTWORK_THREAD_NUM;
};

typedef std::shared_ptr<EdnConfig> EdnConfigPtr;
}

#endif //EDN_EDN_CONFIG_H
