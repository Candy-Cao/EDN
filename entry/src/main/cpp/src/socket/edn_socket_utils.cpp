#include "edn_socket_utils.h"
#include "edn.h"
#include "edn_log.h"


namespace edn {

int SetNonBlocking(int fd)
{
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_opt);
    return old_opt;
}

int Socket(bool use_ipv6)
{
    int fd = socket(use_ipv6 ? AF_INET6 : AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        return -1;
    }
    SetNonBlocking(fd);
    return fd;
}

int ConnectV4(int fd, const sockaddr *addr, socklen_t addrlen)
{
    int ret = connect(fd, addr, addrlen);
    if (ret == -1) {
        if (errno == EINPROGRESS) {
            return EDN_OK;
        } else {
            return EDN_ERR_SYS_ERROR;
        }
    }
    return EDN_OK;
}

int ConnectV4(int fd, const std::string &addr, int port)
{
    sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(port);
    if (inet_pton(AF_INET, addr.c_str(), &addr_in.sin_addr) <= 0) {
        EDN_LOG_ERROR("inet_pton error: %s", strerror(errno));
        return EDN_ERR_SYS_ERROR;
    }
    return ConnectV4(fd, (const sockaddr *)&addr_in, sizeof(addr_in));
}

int ConnectV6(int fd, const std::string &addr, int port)
{
    sockaddr_in6 addr_in6;
    memset(&addr_in6, 0, sizeof(addr_in6));
    addr_in6.sin6_family = AF_INET6;
    addr_in6.sin6_port = htons(port);
    if (inet_pton(AF_INET6, addr.c_str(), &addr_in6.sin6_addr) <= 0) {
        EDN_LOG_ERROR("inet_pton error: %s", strerror(errno));
        return EDN_ERR_SYS_ERROR;
    }
    return ConnectV6(fd, (const sockaddr *)&addr_in6, sizeof(addr_in6));
}

int ConnectV6(int fd, const sockaddr *addr, socklen_t addrlen)
{
    int ret = connect(fd, addr, addrlen);
    if (ret == -1) {
        if (errno == EINPROGRESS) {
            return EDN_OK;
        } else {
            return EDN_ERR_SYS_ERROR;
        }
    }
    return EDN_OK;
}

int Connect(int fd, const std::string ip, int port)
{
    if (ip.find(":") != std::string::npos) {
        return ConnectV6(fd, ip, port);
    } else {
        return ConnectV4(fd, ip, port);
    }
}

int Close(int fd)
{
    if (fd == -1) {
        EDN_LOG_ERROR("socket is invalid.");
        return EDN_ERR_PARAMS_INVALID;
    }
    if (close(fd) == -1) {
        return EDN_ERR_SYS_ERROR;
    }
    return EDN_OK;
}
}