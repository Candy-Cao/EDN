#include "edn_connect.h"
#include "edn_socket_utils.h"
#include "edn_log.h"
#include "cstring"
#include "edn_context.h"

namespace edn {

EdnHashTablePtr EdnConnect::conn_table = std::make_shared<EdnHashTable>();

EdnIOEventCallback EdnConnect::on_conn = [](void* p)->int {
    auto that = (EdnConnect*)p;
    that->SetConnectInfo(GetRealConnectInfo(that->fd_));
    EDN_LOG_INFO("EdnConnect default conn cb, %s", that->Format(that->GetConnectInfo()).c_str());
    that->conn_table->Insert(that->GetConnectInfo(), that->GetFd());
    that->GetContext()->DelEvent(that->GetTimer());
    return EDN_OK;
};

EdnIOEventCallback EdnConnect::on_read = [](void* p)->int {
    EDN_LOG_INFO("EdnConnect default read cb");
    auto that = (EdnConnect*)p;
    auto input = that->input_buffer_;
    int ret = input->RecvAtMost(that->fd_);
    if (ret < 0) {
        EDN_LOG_ERROR("recv data error: %d, errno: %d", ret, errno);
        return ret;
    }
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
        if (!output->GetLowWaterMark() || output->GetSize() < output->GetLowWaterMark()) {
            EDN_LOG_INFO("output buffer low water mark, size: %d", output->GetSize());
            output->CallBack({EDN_OK, 0, std::to_string(ret)});
        }
    }
    if (output->GetSize() == 0) {
        EDN_LOG_INFO("send all data. output buffer is empty");
        that->events_ &= ~EdnEventType::WRITE;//暂时不用监听可写事件
    }
    return EDN_OK;
};

EdnIOEventCallback EdnConnect::on_error = [](void* p)->int {
    EDN_LOG_INFO("EdnConnect default error cb");
    auto that = (EdnConnect*)p;
    int error = 0;
    socklen_t len = sizeof(error);
    if (getsockopt(that->fd_, SOL_SOCKET, SO_ERROR, &error, &len) == -1) {
        EDN_LOG_ERROR("getsockopt error: %d", errno);
        return EDN_ERR_SYS_ERROR;
    }
    EDN_LOG_ERROR("socket error: %d", error);
    return EDN_OK;
};

EdnIOEventCallback EdnConnect::on_close = [](void* p)->int {
    EDN_LOG_INFO("EdnConnect default close cb");
    auto that = (EdnConnect*)p;
    if (that->fd_ != INVALID_FD) {
        close(that->fd_);
        that->fd_ = INVALID_FD;
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

    input_buffer_ = std::make_shared<EdnBuffer>();
    output_buffer_ = std::make_shared<EdnBuffer>();

    connect_info_ = EDN_CONNECT_INFO_INIT;

}

EdnConnect::EdnConnect(const EdnConnectInfo &info)
{
    connect_info_ = EDN_CONNECT_INFO_INIT;
    fd_ = Socket();
    if (fd_ == -1) {
        EDN_LOG_ERROR("Failed to create socket");
        return;
    }
    // Bind local ip&port to a socket
    int ret = Bind(fd_, info.src_ip, info.src_port);
    if (ret != EDN_OK) {
        EDN_LOG_ERROR("Failed to bind local ip&port");
        close(fd_);
        return;
    }
    // Connect to a server
    ret = Connect(fd_, info.dst_ip, info.dst_port);
    if (ret != EDN_OK) {
        EDN_LOG_ERROR("Failed to connect to server");
        close(fd_);
        return;
    }

    conn_cb_ = on_conn;
    read_cb_ = on_read;
    write_cb_ = on_write;
    error_cb_ = on_error;
    close_cb_ = on_close;

    input_buffer_ = std::make_shared<EdnBuffer>();
    output_buffer_ = std::make_shared<EdnBuffer>();
}

EdnConnect::~EdnConnect()
{
    free_connect_info(connect_info_);

    Close();

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
        if (events_ & EdnEventType::WRITE) {//如果已经在监听可写事件，则不需要再添加
            EDN_LOG_INFO("output buffer size: %d", output_buffer_->GetSize());
            return EDN_OK;
        }
        SetEvents(EdnEventType::WRITE);
        auto ret = context_.lock()->GetListener()->add(shared_from_this());//发送完数据后会移除写事件，需要重新监听
        EDN_LOG_DEBUG("add event again, eventId:%d, fd:%d", GetUUID(), fd_);
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
    fd_ = INVALID_FD;
}

int EdnConnect::SetHandleDataCallback(EdnHandleDataCallback cb)
{
    EDN_LOG_DEBUG("enter set handle data cb");
    if (cb) {
        input_buffer_->SetCallback(cb);
    } else {
        EDN_LOG_ERROR("handle data callback is null");
        return EDN_ERR_PARAMS_INVALID;
    }
    return EDN_OK;
}

int EdnConnect::SetMessageEndCallback(EdnMessageEndCallback cb)
{
    EDN_LOG_DEBUG("enter set message end cb");
    if (cb) {
        input_buffer_->SetCallback(cb);
    } else {
        EDN_LOG_ERROR("message end callback is null");
        return EDN_ERR_PARAMS_INVALID;
    }
    return EDN_OK;
}

std::string EdnConnect::Format(const EdnConnectInfo &info)
{
    std::string result("Connect info:\n");
    if (info.src_ip) {
        result += "src_ip: " + std::string(info.src_ip) + "\n";
    }
    if (info.src_port) {
        result += "src_port: " + std::to_string(info.src_port) + "\n";
    }
    if (info.dst_ip) {
        result += "dst_ip: " + std::string(info.dst_ip) + "\n";
    }
    if (info.dst_port) {
        result += "dst_port: " + std::to_string(info.dst_port) + "\n";
    }
    return result;
}

void EdnConnect::SetConnectInfo(const EdnConnectInfo &info)
{
    connect_info_ = copy_connect_info(ConnectInfoNomalize(info));

}

EdnConnectInfo EdnConnect::GetConnectInfo() const
{
    return connect_info_;
}

void EdnConnect::SetTimer(EdnTimerPtr timer)
{
    timer_ = timer;
}

EdnTimerPtr EdnConnect::GetTimer() const
{
    return timer_;
}

EdnConnectInfo EdnConnect::ConnectInfoNomalize(const EdnConnectInfo &info)
{
    auto copy = info;
    if (copy.src_ip == nullptr || copy.src_ip[0] == '\0') {
        if (IsIpV6(info.dst_ip)) {
            copy.src_ip = GetDefaultIpV6().c_str();
        } else {
            copy.src_ip = GetDefaultIpV4().c_str();
        }
    }
    else if (std::string(copy.src_ip) == DEFAULT_IVP4_ADDR) {
        copy.src_ip = GetDefaultIpV4().c_str();
    }
    else if (std::string(copy.src_ip) == DEFAULT_IVP6_ADDR) {
        copy.src_ip = GetDefaultIpV6().c_str();
    }
    return copy;
}
}