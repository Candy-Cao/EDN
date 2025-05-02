#include "edn_hash_table.h"
#include "edn_log.h"
#include "edn_define.h"

namespace edn {
void EdnHashTable::Insert(const EdnConnectInfo &info, int fd)
{
    std::lock_guard<std::mutex> lock(mutex_);
    hash_table_[info] = fd;
}

int EdnHashTable::Get(const EdnConnectInfo &info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = hash_table_.find(info);
    if (it != hash_table_.end()) {
        return it->second;
    }
    return INVALID_FD; // Not found
}

void EdnHashTable::Remove(const EdnConnectInfo &info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = hash_table_.find(info);
    if (it == hash_table_.end()) {
        EDN_LOG_WARN("Connection not found in hash table");
        return;
    }
    hash_table_.erase(info);
}

void EdnHashTable::Clear()
 {
        std::lock_guard<std::mutex> lock(mutex_);
        hash_table_.clear();
    }
}