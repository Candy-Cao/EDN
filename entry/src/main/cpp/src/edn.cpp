#include "edn.h"
#include "edn_log.h"
#include "edn_connect.h"
#include "edn_context.h"
#include "edn_define.h"
#include "edn_hash_table.h"
#include <cstring>
#include "cstdarg"

using namespace edn;

EdnConnetInfo copy_connect_info(EdnConnetInfo info)
{
    return {strdup(info.src_ip), info.src_port, strdup(info.dst_ip), info.dst_port};
}

void free_connect_info(EdnConnetInfo info)
{
    if (info.src_ip != nullptr) {
        free((void*)info.src_ip);
    }
    if (info.dst_ip != nullptr) {
        free((void*)info.dst_ip);
    }
    info.src_ip = nullptr;
    info.dst_ip = nullptr;
}

void edn_run()
{
    auto ctx = EdnContext::GetInstance();
    ctx->Run();
    EDN_LOG_INFO("edn run");
}

int32_t edn_connect(EdnConnetInfo info, int timeout, pf_opt_complete_cb cb) {
    info = EdnConnect::ConnectInfoNomalize(info);
    int fd = EdnConnect::conn_table->Get(info);
    if (fd != -1) {
        EDN_LOG_WARN("connect already exist, connect_id: %d", fd);
        return fd;
    }
    auto conn = std::make_shared<EdnConnect>(info.dst_ip, info.dst_port);
    auto ctx = EdnContext::GetInstance();
    ctx->AddEvent(conn);
    auto timer = std::make_shared<EdnTimer>(timeout, false);
    timer->SetCallback([=]() {
        EDN_LOG_ERROR("connect timeout");
        conn->Close();
        cb(EDN_ERR_TIMEOUT);
        EdnContext::GetInstance()->DelEvent(conn);
    });
    conn->SetTimer(timer);
    return conn->GetFd();
}

EDN_CODE edn_send_data(int32_t connect_id, const char *buffer, int len, pf_opt_complete_cb cb) {
    if (buffer == nullptr || len <= 0) {
        EDN_LOG_ERROR("send data error, data is null or len is 0");
        return EDN_ERR_PARAMS_INVALID;
    }
    EDN_LOG_DEBUG("send data, connect_id: %d, len: %d", connect_id, len);
    auto ctx = EdnContext::GetInstance();
    auto it = std::dynamic_pointer_cast<EdnConnect>(ctx->GetEvent(connect_id));
    it->SendData(buffer, len, [cb] (EdnError err) {
        if (err.edn_code != EDN_OK) {
            EDN_LOG_ERROR("send data error: %d, errno: %d", err.edn_code, err.sys_code);
        }
        cb(err.edn_code);
    });
    return EDN_OK;
}

EDN_CODE edn_set_opt(int32_t connect_id, EDN_OPT_TYPE type, ...) {

    va_list args;
    va_start(args, type);
    auto ctx = EdnContext::GetInstance();
    auto it = std::dynamic_pointer_cast<EdnConnect>(ctx->GetEvent(connect_id));
    if (it == nullptr) {
        EDN_LOG_ERROR("event not find, connect_id: %d", connect_id);
        return EDN_ERR_PARAMS_INVALID;
    }
    int ret = 0;
    switch (type) {
        case EDN_OPT_TYPE_HANDLE_DATA_CB: {
            auto cb = va_arg(args, pf_handle_data_cb);
            ret = it->SetHandleDataCallback(cb);
            EDN_LOG_DEBUG("set handle data cb, ret: %d", ret);
            break;
        }
        case EDN_OPT_TYPE_MESSAGE_END_CB: {
            auto cb = va_arg(args, pf_message_package_end_cb);
            ret = it->SetMessageEndCallback(cb);
            EDN_LOG_DEBUG("set message end cb, ret: %d", ret);
            break;
        }
        default:
            EDN_LOG_ERROR("set option error, type: %d", type);
            return EDN_ERR_PARAMS_INVALID;
    }
    va_end(args);
    EDN_LOG_INFO("set option success, connect_id: %d", connect_id);
    return EDN_OK;
}

EDN_CODE edn_close(int32_t connect_id) {
    auto ctx = EdnContext::GetInstance();
    auto it = std::dynamic_pointer_cast<EdnConnect>(ctx->GetEvent(connect_id));
    if (it == nullptr) {
        EDN_LOG_ERROR("event not find, connect_id: %d", connect_id);
        return EDN_ERR_PARAMS_INVALID;
    }
    it->Close();
    ctx->DelEvent(it);
    EDN_LOG_INFO("close connect, connect_id: %d", connect_id);
    return EDN_OK;
}

void edn_set_log_cb(pf_log_cb cb) {
    EdnLogger::GetInstanse()->SetLogCb(cb);
}
