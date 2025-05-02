/**
 * Copyright 2025 The EDN Authors
 * All rights reserved.
 * created by candy
 * 2025/3/16
 * @brief hash table
 */

#ifndef EDN_HASH_TABLE_H
#define EDN_HASH_TABLE_H

#include "edn.h"
#include <string>
#include <cstring>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace std {//特化EdnConnectInfo的hash函数
    template<>
    struct hash<EdnConnectInfo> {
        size_t operator()(const EdnConnectInfo& info) const {
            return std::hash<std::string>()(info.dst_ip) ^ std::hash<uint32_t>()(info.dst_port) ^
                   std::hash<std::string>()(info.src_ip) ^ std::hash<uint32_t>()(info.src_port);
        }
    };

}

//重载 EdnConnectInfo 的 == 运算符
inline bool operator==(const EdnConnectInfo& lhs, const EdnConnectInfo& rhs) {
    return strcmp(lhs.dst_ip, rhs.dst_ip) == 0 && lhs.dst_port == rhs.dst_port &&
           strcmp(lhs.src_ip, rhs.src_ip) == 0 && lhs.src_port == rhs.src_port;
}

namespace edn {


class EdnHashTable {    
public:
    EdnHashTable() = default;
    ~EdnHashTable() = default;

    void Insert(const EdnConnectInfo& info, int fd);

    int Get(const EdnConnectInfo& info);

    void Remove(const EdnConnectInfo& info);

    void Clear();

private:
    std::unordered_map<EdnConnectInfo, int> hash_table_;
    std::mutex mutex_;
};

typedef std::shared_ptr<EdnHashTable> EdnHashTablePtr;

}

#endif //EDN_HASH_TABLE_H