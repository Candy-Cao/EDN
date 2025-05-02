#include "edn.h"
#include "edn_log.h"
#include "edn_timer.h"
#include "edn_connect.h"
#include "edn_context.h"
#include <unistd.h>
#include <cstring>


using namespace edn;

int32_t handle_data_cb(const char* data, int len) {
    EDN_LOG_DEBUG("test handle data cb, len: %d", len);
    if (len > 0) {
        char* tmp = new char[len + 1];
        memcpy(tmp, data, len);
        tmp[len] = '\0';
        EDN_LOG_DEBUG("test handle len:%d data: %s", len, tmp);
        delete[] tmp;
    } else {
        EDN_LOG_INFO("handle data is empty");
    }
    return len;
}

size_t message_end_cb(const char* data, int len) {
    EDN_LOG_DEBUG("test message end cb, len: %d", len);
    return len;
}

int main() {

    // Create a connection info structure
    EdnConnectInfo conn_info = EDN_CONNECT_INFO_INIT;
    conn_info.dst_ip = "127.0.0.1";
    conn_info.dst_port = 8080;
    auto conn_id = edn_connect(conn_info, 5000, [](int32_t error) {
        if (error != EDN_OK) {
            EDN_LOG_ERROR("Connection error: %d", error);
        } else {
            EDN_LOG_INFO("Connection established successfully");
        }
    });
    if (conn_id < 0) {
        EDN_LOG_ERROR("Failed to create connection");
        return -1;
    }
    // Set the handle data callback
    edn_set_opt(conn_id, EDN_OPT_TYPE_HANDLE_DATA_CB, handle_data_cb);
    // Set the message end callback
    edn_set_opt(conn_id, EDN_OPT_TYPE_MESSAGE_END_CB, message_end_cb);
    // Send data
    auto timer = std::make_shared<EdnTimer>(1000, true);
    timer->SetCallback([=]() {
        const char* msg = "Hello, server!\n";
        edn_send_data(conn_id, msg, strlen(msg), [](int32_t error) {
            if (error != EDN_OK) {
                EDN_LOG_ERROR("Send data error: %d", error);
            } else {
                EDN_LOG_INFO("Data sent successfully");
            }
        });
    });
    auto ctx = EdnContext::GetInstance();
    ctx->AddEvent(timer);
    // Run the event loop   
    edn_run();

}