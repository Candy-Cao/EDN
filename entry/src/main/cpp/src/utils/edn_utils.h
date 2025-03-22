//
// Created on 2025/3/14.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef EDN_EDN_UTILS_H
#define EDN_EDN_UTILS_H

namespace edn {


class EdnUtils {
public:
    static int GetUUID();
    
    //设置非阻塞标志
    static int SetNonBlocking(int fd);
};
}
#endif //EDN_EDN_UTILS_H
