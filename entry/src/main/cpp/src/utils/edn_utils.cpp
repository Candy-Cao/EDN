//
// Created on 2025/3/14.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "edn_utils.h"
#include <mutex>
#include <fcntl.h>
#include <sys/time.h>

namespace edn {
static std::mutex g_uuid_mutex;
int EdnUtils::GetUUID() {
    std::lock_guard<std::mutex> lock(g_uuid_mutex);
    static int s_uuid = 0;
     ++s_uuid;
    return s_uuid;
}

int EdnUtils::GetCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
}