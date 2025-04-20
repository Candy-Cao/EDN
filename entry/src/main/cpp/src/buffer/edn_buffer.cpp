#include "edn_buffer.h"
#include "edn_define.h"
#include "edn_log.h"
#include "edn_config.h"
#include "singleton.h"
#include <cstring>
#include <cassert>

namespace edn {

void BufferNode::Init(size_t len) {
    assert(len > 0);
    this->len = 0;
    this->data = (char*)this + sizeof(BufferNode);
    this->capacity = len - sizeof(BufferNode);
    this->offset = 0;
}

void BufferNode::Write(const char *data, size_t len)
{
    assert(len > 0 && data != nullptr);
    memcpy(this->data + offset, data, len);
    this->len += len;
    EDN_LOG_INFO("write %zu bytes to buffer node", len);
    return;
}

EdnBuffer::EdnBuffer()
{
    int capacity = Singleton<EdnConfig>::getInstance()->buffer_len;
    buffer_ = new char[capacity];
    size_ = 0;
    capacity_ = capacity;
    head_ = nullptr;
    tail_ = nullptr;
}

EdnBuffer::~EdnBuffer()
{
    if (buffer_) {
        delete[] buffer_;
        buffer_ = nullptr;
    }
}

int EdnBuffer::Write(const char *_data, size_t _len)
{
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    auto real_len = std::max(_len, (size_t)Singleton<EdnConfig>::getInstance()->min_buffer_node_len);
    if (real_len > capacity_ - size_) {
        EDN_LOG_ERROR("buffer is not enough");
        return EDN_ERR_BUFFER_NOT_ENOUGH;
    }
    if (tail_ == nullptr) {
        tail_ = (BufferNode*)buffer_;
        
        tail_->Init(real_len);
        tail_->Write(_data, _len);
        head_ = tail_;
        size_ += _len;
        buffer_node_num_ = 1;
        EDN_LOG_INFO("first write %zu bytes to buffer", _len);
        return EDN_OK;
    }
    else if (tail_->RemainSpace() >= _len) {
        tail_->Write(_data, _len);
        size_ += _len;
        EDN_LOG_INFO("write %zu bytes to buffer", _len);
        return EDN_OK;
    }
    else {
        const char *rdata = _data + tail_->RemainSpace();
        size_t rlen = _len - tail_->RemainSpace();
        tail_->Write(_data, tail_->RemainSpace());
        
        auto* node = __GetNextBufferNode(std::max(rlen, (size_t)Singleton<EdnConfig>::getInstance()->min_buffer_node_len));
        if (node == nullptr) {
            EDN_LOG_ERROR("buffer is not enough");
            return EDN_ERR_BUFFER_NOT_ENOUGH;
        }
        node->Init(real_len);
        node->Write(rdata, rlen);
        tail_->next = node;
        node->prev = tail_;
        tail_ = node;
        size_ += _len;
        buffer_node_num_++;
        EDN_LOG_INFO("add node, write %zu bytes to buffer", _len);
        return EDN_OK;
    }
    return EDN_OK;
}

int EdnBuffer::SendAtMost(int fd)
{
    int num = 0;
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    if (head_ == nullptr) {
        return 0;
    }
    BufferNode *node = head_;
    while (node != nullptr) {
        if (node->len > 0) {
            ssize_t ret = send(fd, node->data + node->offset, node->len, 0);
            if (ret == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    EDN_LOG_INFO("send data would block");
                    return num;
                }
                else if (errno == EINTR) {
                    EDN_LOG_INFO("send data interrupted");
                    continue;
                }
                else {
                    EDN_LOG_ERROR("send data error: %d", errno);
                    return EDN_ERR_SYS_ERROR;
                }
            }
            num += ret;
            node->offset += ret;
            node->len -= ret;
            size_ -= ret;
            if (node->len == 0) {
                head_ = node->next;
                if (head_) {
                    head_->prev = nullptr;
                }
                else {
                    tail_ = nullptr;
                }
                buffer_node_num_--;
                node = node->next;
            }
        }
    }
    return num;
}

BufferNode *EdnBuffer::__GetNextBufferNode(size_t _len) 
{
    if (tail_ == nullptr) {
        return (BufferNode*)buffer_;
    }
    else if (head_ < tail_ && tail_->data + tail_->capacity + _len <= buffer_ + capacity_) {
        return tail_ + 1;
    }
    else if (head_ < tail_ && (BufferNode*)(buffer_ + _len) < head_) {
        return (BufferNode*)buffer_;
    }
    else if (tail_ < head_ && (BufferNode*)(tail_->data + tail_->capacity + _len) <= head_) {
        return tail_ + 1;
    }
    return nullptr;
}
}