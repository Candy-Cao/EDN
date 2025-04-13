#include "edn_io_event.h"
#include "edn.h"
#include "edn_log.h"
#include "edn_context.h"
#include "edn_utils.h"
#include "edn_socket_utils.h"
#include "singleton.h"

using namespace edn;

int main() {
    // Create a socket
    int fd = Socket();
    if (fd == -1) {
        EDN_LOG_ERROR("Failed to create socket");
        return -1;
    }
    // Connect to a server
    int ret = Connect(fd, "127.0.0.1", 8080);
    if (ret != EDN_OK) {
        EDN_LOG_ERROR("Failed to connect to server");
        close(fd);
        return -1;
    }
    // Create an IO event
    auto io_event = std::make_shared<EdnIOEvent>(fd, true);
    auto ctx = Singleton<EdnContext>::getInstance();
    ctx->AddEvent(io_event);
    io_event->SetConnCb([](void* p) {
        EDN_LOG_INFO("Connect callback triggered*******");
        // Handle write event
        auto that = (EdnIOEvent*)p;
        const char* msg = "Hello, server!";
        ssize_t bytes_written = send(that->GetFd(), msg, strlen(msg), 0);
        if (bytes_written == -1) {
            EDN_LOG_ERROR("Failed to send message: %s", strerror(errno));
            return EDN_ERR_SYS_ERROR;
        }
        EDN_LOG_INFO("Sent %zd bytes to server", bytes_written);
        return EDN_OK;
    });
    io_event->SetReadCb([](void* p) {
        EDN_LOG_INFO("Read callback triggered*******");
        // Handle read event
        auto that = (EdnIOEvent*)p;
        char buffer[1024];
        ssize_t bytes_read = recv(that->GetFd(), buffer, sizeof(buffer), 0);
        if (bytes_read == -1) {
            EDN_LOG_ERROR("Failed to receive message: %s", strerror(errno));
            return EDN_ERR_SYS_ERROR;
        }
        buffer[bytes_read] = '\0';
        EDN_LOG_INFO("Received %zd bytes from server: %s", bytes_read, buffer);

        that->GetContext()->DelEvent(that->shared_from_this());
 
        return EDN_OK;
    });
    ctx->Run();
    return 0;
}