#include "edn_buffer.h"
#include "edn_define.h"
#include "edn_log.h"
#include "edn_config.h"
#include "edn_socket_utils.h"
#include "singleton.h"
#include <cstring>
#include <cassert>
#include <errno.h>


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
    EDN_LOG_DEBUG("write %zu bytes to buffer node", len);
    return;
}

int32_t EdnBuffer::DefaultHandleDataCb(const char* data, int len) {
    EDN_LOG_DEBUG("default handle data cb");
    if (len > 0) {
        std::string tmp(data, len);
        EDN_LOG_DEBUG("handle len:%d data: %s", len, tmp.c_str());
    } else {
        EDN_LOG_INFO("handle data is empty");
    }
    return len;
}

size_t EdnBuffer::DefaultMessageEndCb(const char* data, int len) {
    EDN_LOG_DEBUG("default message end cb, len: %d", len);
    return len;
}

EdnBuffer::EdnBuffer()
{
    int capacity = Singleton<EdnConfig>::getInstance()->buffer_len;
    buffer_ = new char[capacity];
    size_ = 0;
    capacity_ = capacity;
    head_ = nullptr;
    tail_ = nullptr;
    handle_data_cb_ = &DefaultHandleDataCb;
    message_end_cb_ = &DefaultMessageEndCb;
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
    EDN_LOG_DEBUG("write %zu bytes to buffer", _len);
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    auto node_len = std::max(_len + sizeof(BufferNode), (size_t)Singleton<EdnConfig>::getInstance()->min_buffer_node_len);
    if (node_len > capacity_ - size_) {
        EDN_LOG_ERROR("buffer is not enough");
        return EDN_ERR_BUFFER_NOT_ENOUGH;
    }
    if (tail_ == nullptr) {
        tail_ = (BufferNode*)buffer_;
        
        tail_->Init(node_len);
        tail_->Write(_data, _len);
        head_ = tail_;
        size_ += _len;
        buffer_node_num_ = 1;
        EDN_LOG_DEBUG("first write %zu bytes to buffer", _len);
        return EDN_OK;
    }
    else if (tail_->RemainSpace() >= _len) {
        tail_->Write(_data, _len);
        size_ += _len;
        EDN_LOG_DEBUG("write %zu bytes to buffer", _len);
        return EDN_OK;
    }
    else {
        const char *rdata = _data + tail_->RemainSpace();
        size_t rlen = _len - tail_->RemainSpace();
        tail_->Write(_data, tail_->RemainSpace());
        
        auto* node = __GetNextBufferNode(std::max(rlen + sizeof(BufferNode), (size_t)Singleton<EdnConfig>::getInstance()->min_buffer_node_len));
        if (node == nullptr) {
            EDN_LOG_ERROR("buffer is not enough");
            return EDN_ERR_BUFFER_NOT_ENOUGH;
        }
        node->Init(node_len);
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

int EdnBuffer::RecvAtMost(int fd)
{
    int num = 0;
    int n = GetReadableBytes(fd);//参考内存块大小用于读取fd数据
    EDN_LOG_DEBUG("socket data from fd: %d, readable bytes: %d", fd, n);
    int node_len = std::max<size_t>(n + sizeof(BufferNode), (int)Singleton<EdnConfig>::getInstance()->min_buffer_node_len);
    if (node_len > capacity_ - size_ - sizeof(BufferNode) * buffer_node_num_) {
        EDN_LOG_ERROR("buffer is not enough, need more large buffer");
        return EDN_ERR_BUFFER_NOT_ENOUGH;
    }
    int ret = -1;
    int max_read_len = 0;//单次读取数据长度

    std::lock_guard<std::mutex> lock(buffer_mutex_);
    auto node = tail_;
    do {
        if (node == nullptr) {
            node = (BufferNode*)buffer_;
            node->Init(node_len);
            head_ = node;
            tail_ = node;
            max_read_len = node->capacity;
        }

        max_read_len = node->RemainSpace();
        int flags = fcntl(fd, F_GETFL, 0);
        EDN_LOG_DEBUG("begin recv data, max read len: %d, O_NONBLOCK:%d", max_read_len, flags & O_NONBLOCK);
        ret = recv(fd, node->data + node->offset + node->len, max_read_len, MSG_DONTWAIT);
        EDN_LOG_DEBUG("recv data from fd: %d, ret: %d, content:%s", fd, ret, ret > 0 ? std::string(node->data + node->offset + node->len, ret).c_str(): "");
        if (ret == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                EDN_LOG_INFO("all data received ,recv data would block");
                break;
            }
            else if (errno == EINTR) {
                EDN_LOG_INFO("recv data interrupted");
                continue;
            }
            else {
                EDN_LOG_ERROR("recv data error: %d", errno);
                return EDN_ERR_SYS_ERROR;
            }
        }
        if (ret == 0) {
            EDN_LOG_WARN("peer closed");
            return 0;
        }
        num += ret;
        size_ += ret;
        node->len += ret;
        if (node->len == node->capacity) {
            node = __GetNextBufferNode(node_len);
            if (node == nullptr) {
                EDN_LOG_ERROR("buffer is not enough");
                return EDN_ERR_BUFFER_NOT_ENOUGH;
            }
            node->Init(node_len);
            tail_->next = node;
            node->prev = tail_;
            tail_ = node;
            buffer_node_num_++;
        }
        n = GetReadableBytes(fd);
    } while (true);
    //消费数据
    EDN_LOG_DEBUG("recv data end, callback data, size: %d", size_);
    char *tmp_buf = new char[size_];
    int len = 0;
    int offset = 0;
    node = head_;
    while (node != nullptr) {
        memcpy(tmp_buf + offset + len, node->data + node->offset, node->len);
        len += node->len;
        int msglen = message_end_cb_(tmp_buf + offset, len);
        while (msglen > 0) {//消息包结束标志
            EDN_LOG_DEBUG("begin run handle data cb, msglen: %d", msglen);
            msglen = handle_data_cb_(tmp_buf + offset, msglen);
            offset += msglen;
            len -= msglen;
            __PopData(msglen);
            msglen = message_end_cb_(tmp_buf + offset, len);
        }
        node = node->next;
    }
    delete[] tmp_buf;
    return num;
}

void EdnBuffer::SetCallback(EdnAsyncOptCallback cb)
{
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    cb_ = cb;
}

void EdnBuffer::CallBack(EdnError err)
{
    EDN_LOG_DEBUG("Enter EdnBuffer::CallBack");
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    if (cb_) {
        EDN_LOG_DEBUG("begin call EdnAsyncOptCallback");
        cb_(err);
    }
}

void EdnBuffer::SetCallback(EdnHandleDataCallback cb)
{
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    handle_data_cb_ = cb;
}

void EdnBuffer::SetCallback(EdnMessageEndCallback cb)
{
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    message_end_cb_ = cb;
}

int EdnBuffer::GetSize()
{
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    return size_;
}

int EdnBuffer::GetCapacity()
{
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    return capacity_;
}

int EdnBuffer::GetHighWaterMark()
{
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    return high_water_mark_;
}

int EdnBuffer::GetLowWaterMark()
{
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    return low_water_mark_;
}
int EdnBuffer::SendAtMost(int fd)
{
    EDN_LOG_DEBUG("send data to fd: %d", fd);
    int num = 0;
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    if (head_ == nullptr) {
        return 0;
    }
    BufferNode *node = head_;
    while (node != nullptr) {
        if (node->len > 0) {
            ssize_t ret = send(fd, node->data + node->offset, node->len, 0);
            EDN_LOG_DEBUG("send data frame, len: %zu, offset: %zu, ret:%d", node->len, node->offset, ret);
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

void EdnBuffer::__PopData(int len)
{
    auto node = head_;
    while (len) {
        if (node->len > len) {
            node->len -= len;
            node->offset += len;
            len = 0;
            size_ -= len;
        }
        else if (node->len == len) {
            head_ = node->next;
            if (head_) {
                head_->prev = nullptr;
            }
            else {
                tail_ = nullptr;
            }
            buffer_node_num_--;
            size_ -= node->len;   
            len = 0;
        }
        else {
            len -= node->len;
            size_ -= node->len;
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
}