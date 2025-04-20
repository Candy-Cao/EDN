/**
 * @file edn_buffer.h
 * Copyright 2023 The Edn Authors
 * All rights reserved.
 * created by candy
 * 2025/3/16
 * @brief buffer
 */

#ifndef EDN_BUFFER_H
#define EDN_BUFFER_H

#include <mutex>
namespace edn {

typedef std::function<void(EdnError)> EdnAsyncOptCallback;


struct BufferNode {
    BufferNode* next = nullptr;
    BufferNode* prev = nullptr;
    
    size_t len = 0;         //数据长度
    size_t offset = 0;      //数据偏移量
    size_t capacity = 0;    //数据容量
    char* data = nullptr;

    void Init(size_t len);

    void Write(const char* data, size_t len);

    inline int RemainSpace() {
        return capacity - len - offset;
    }


};

class EdnBuffer {
public:
    EdnBuffer() ;
    ~EdnBuffer() = default;

    int Write(const char* data, size_t len);
    /**
     * * @brief 发送尽可能多的数据
     * * @param fd 文件描述符
     * * @return 发送的字节数或错误码（<0）
     */
    int SendAtMost(int fd);

    void SetCallback(EdnAsyncOptCallback cb) {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        cb_ = cb;
    }
    void CallBack(EdnError err) {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        if (cb_) {
            cb_(err);
        }
    }
    int GetSize() {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        return size_;
    }
    int GetCapacity() {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        return capacity_;
    }
    int GetHighWaterMark() {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        return high_water_mark_;
    }
    int GetLowWaterMark() {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        return low_water_mark_;
    }
private:

    BufferNode* __GetNextBufferNode(size_t len);

private:

    char* buffer_;         //缓冲区
    size_t size_;           //当前缓冲区使用量
    size_t capacity_;       //缓冲区容量
    EdnAsyncOptCallback cb_;
    int high_water_mark_ = 0; //缓冲区高水位线
    int low_water_mark_ = 0;  //缓冲区低水位线
    BufferNode* head_ = nullptr; //缓冲区头节点
    BufferNode* tail_ = nullptr; //缓冲区尾节点
    std::mutex buffer_mutex_; //缓冲区锁
    int buffer_node_num_ = 0; //缓冲区节点数
};

}
#endif