#include "edn_connect.h"
#include "edn_socket_utils.h"
#include "edn_log.h"
#include "cstring"
#include "edn_context.h"

namespace edn {

EdnIOEventCallback EdnConnect::on_conn = [](void* p)->int {
    EDN_LOG_INFO("EdnConnect default conn cb");
    return EDN_OK;
};

EdnIOEventCallback EdnConnect::on_read = [](void* p)->int {
    EDN_LOG_INFO("EdnConnect default read cb");

    return EDN_OK;
};

EdnIOEventCallback EdnConnect::on_write = [](void* p)->int {
    EDN_LOG_INFO("EdnConnect default write cb");
    auto that = (EdnConnect*)p;
    auto output = that->output_buffer_;
    if (output->GetSize() > 0) {
        int ret = output->SendAtMost(that->fd_);
        if (ret < 0) {
            EDN_LOG_ERROR("send data error: %d, errno: %d", ret, errno);
            output->CallBack({ret, errno, strerror(errno)});
            return ret;
        }
        if (output->GetLowWaterMark() && output->GetSize() < output->GetLowWaterMark()) {
            EDN_LOG_INFO("output buffer low water mark, size: %d", output->GetSize());
            output->CallBack({EDN_OK, 0, std::to_string(ret)});
        }
        if (output->GetSize() == 0) {
            EDN_LOG_INFO("send all data. output buffer is empty");
            that->events_ &= ~EdnEventType::WRITE;//暂时不用监听可写事件
        }
    }
    return EDN_OK;
};

EdnConnect::EdnConnect(const std::string &ip, int port)
{
    // Create a socket
    int fd = Socket();
    if (fd == -1) {
        EDN_LOG_ERROR("Failed to create socket");
        return;
    }
    fd_ = fd;
    // Connect to a server
    int ret = Connect(fd, ip, port);
    if (ret != EDN_OK) {
        EDN_LOG_ERROR("Failed to connect to server");
        close(fd);
        return;
    }

    conn_cb_ = on_conn;
    read_cb_ = on_read;
    write_cb_ = on_write;
    error_cb_ = on_error;
    close_cb_ = on_close;

}

int EdnConnect::SendData(const char *data, size_t len, EdnAsyncOptCallback cb)
{
    if (data == nullptr || len <= 0) {
        EDN_LOG_ERROR("send data error, data is null or len is 0");
        return EDN_ERR_PARAMS_INVALID;
    }
    auto ret = output_buffer_->Write(data, len);
    if (ret != EDN_OK) {
        EDN_LOG_ERROR("write data to buffer failed, error: %d", ret);
        return ret;
    }
    output_buffer_->SetCallback(cb);
    if (output_buffer_->GetSize() > 0) {
        if (events_ & EdnEventType::WRITE) {
            EDN_LOG_INFO("output buffer size: %d", output_buffer_->GetSize());
            return EDN_OK;
        }
        events_ |= EdnEventType::WRITE;
        auto ret = context_.lock()->GetListener()->add(shared_from_this());
        if (ret != EDN_OK) {
            EDN_LOG_ERROR("add event error: %d", ret);
            return ret;
        }
    }
    return EDN_OK;
}

int EdnConnect::Close()
{
    return edn::Close(fd_);
}

}