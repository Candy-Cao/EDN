//
// Created on 2025/3/14.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef EDN_EDN_UTILS_H
#define EDN_EDN_UTILS_H
#include <string>

namespace edn {


class EdnUtils {
public:
    static int GetUUID();
    

    static int GetCurrentTime();

    static std::string GetTimeStamp();
};
}
#endif //EDN_EDN_UTILS_H
