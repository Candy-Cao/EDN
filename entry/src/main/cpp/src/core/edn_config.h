//
// Created on 2025/3/16.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef EDN_EDN_CONFIG_H
#define EDN_EDN_CONFIG_H

#include "edn_define.h"
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
    int buffer_len = DEFAULT_BUFFER_LEN;
    int min_buffer_node_len = DEFAULT_MIN_BUFFER_NODE_LEN;
    int write_high_water_mark = DEFAULT_WRITE_HIGH_WATER_MARK;
    int write_low_water_mark = DEFAULT_WRITE_LOW_WATER_MARK;
    int read_high_water_mark = DEFAULT_READ_HIGH_WATER_MARK;
    int read_low_water_mark = DEFAULT_READ_LOW_WATER_MARK;
};

typedef std::shared_ptr<EdnConfig> EdnConfigPtr;
}

#endif //EDN_EDN_CONFIG_H
