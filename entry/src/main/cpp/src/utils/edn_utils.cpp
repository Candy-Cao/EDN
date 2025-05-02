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
// 获取当前时间戳
// 返回格式：2023-10-01 12:00:00.000
std::string EdnUtils::GetTimeStamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t now = tv.tv_sec;
    struct tm *tm_now = localtime(&now);
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
             tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday,
             tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec, tv.tv_usec / 1000);
    return std::string(buffer);
}
}