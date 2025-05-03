#include "edn_socket_utils.h"
#include "edn.h"
#include "edn_log.h"
#include "edn_define.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>


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

int Bind(int fd, const sockaddr *addr, socklen_t addrlen)
{
    int ret = bind(fd, addr, addrlen);
    if (ret == -1) {
        EDN_LOG_ERROR("bind error: %s", strerror(errno));
        return EDN_ERR_SYS_ERROR;
    }
    return EDN_OK;
}

int BindV4(int fd, const std::string &ip, int port)
{
    if (ip.empty() && ip == DEFAULT_IVP4_ADDR) {
        return EDN_OK;
    }
    sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr_in.sin_addr) <= 0) {
        EDN_LOG_ERROR("inet_pton error: %d, ip:%s, port:%d", errno, ip.c_str(), port);
        return EDN_ERR_SYS_ERROR;
    }
    return Bind(fd, (const sockaddr *)&addr_in, sizeof(addr_in));
}

int BindV6(int fd, const std::string &addr, int port)
{
    if (addr.empty() && addr == DEFAULT_IVP6_ADDR) {
        return EDN_OK;
    }
    sockaddr_in6 addr_in6;
    memset(&addr_in6, 0, sizeof(addr_in6));
    addr_in6.sin6_family = AF_INET6;
    addr_in6.sin6_port = htons(port);
    if (inet_pton(AF_INET6, addr.c_str(), &addr_in6.sin6_addr) <= 0) {
        EDN_LOG_ERROR("inet_pton error: %s, ip:%s, port:%d", strerror(errno), addr.c_str(), port);
        return EDN_ERR_SYS_ERROR;
    }
    return Bind(fd, (const sockaddr *)&addr_in6, sizeof(addr_in6));
}

int Bind(int fd, const std::string &ip, int port)
{
    if (ip.find(":") != std::string::npos) {
        return BindV6(fd, ip, port);
    } else {
        return BindV4(fd, ip, port);
    }
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
    EDN_LOG_INFO("connect :%d closed.", fd);
    if (fd == INVALID_FD) {
        EDN_LOG_ERROR("socket is invalid.");
        return EDN_ERR_PARAMS_INVALID;
    }
    if (close(fd) == -1) {
        return EDN_ERR_SYS_ERROR;
    }
    return EDN_OK;
}

int GetReadableBytes(int fd)
{
#if defined(FIONREAD)
	int n = DEFAULT_MIN_BUFFER_NODE_LEN;
	if (ioctl(fd, FIONREAD, &n) < 0)
		return -1;
	return n;
#else
    return DEFAULT_MIN_BUFFER_NODE_LEN;
#endif
}

std::string GetDefaultIpV4()
{
    int sockfd;
    struct ifreq ifr;

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        EDN_LOG_ERROR("create socket error: %s", strerror(errno));
        return DEFAULT_IVP4_ADDR;
    }

    // 获取默认网卡名称
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1); // 默认尝试 eth0
    if (ioctl(sockfd, SIOCGIFADDR, &ifr) == 0) {
        struct sockaddr_in* ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
        EDN_LOG_INFO("Default interface: %s\n", ifr.ifr_name);
        EDN_LOG_INFO("IP address: %s\n", inet_ntoa(ipaddr->sin_addr));
    } else {
        EDN_LOG_ERROR("ioctl");
        return DEFAULT_IVP4_ADDR;
    }

    close(sockfd);
    return inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr);
}

std::string GetDefaultIpV6()
{
    struct ifaddrs *ifaddr, *ifa;
    char ip[INET6_ADDRSTRLEN] = DEFAULT_IVP6_ADDR;

    // 获取所有网络接口
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return DEFAULT_IVP6_ADDR;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        // 检查是否为 IPv6 地址
        if (ifa->ifa_addr->sa_family == AF_INET6) {
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *)ifa->ifa_addr;

            // 排除本地地址 (::1)
            if (!IN6_IS_ADDR_LOOPBACK(&addr->sin6_addr)) {
                inet_ntop(AF_INET6, &addr->sin6_addr, ip, sizeof(ip));
                EDN_LOG_INFO("Interface: %s, IP: %s" , ifa->ifa_name, ip);
                break; // 找到第一个非本地 IPv6 地址后退出
            }
        }
    }

    freeifaddrs(ifaddr);
    return std::string(ip);
}

EdnConnectInfo GetRealConnectInfo(int fd, bool is_ipv6)
{
    EdnConnectInfo info = EDN_CONNECT_INFO_INIT;
    if (is_ipv6) {
        sockaddr_in6 addr;
        socklen_t addrlen = sizeof(addr);
        if (getpeername(fd, (struct sockaddr *)&addr, &addrlen) == -1) {
            EDN_LOG_ERROR("getpeername error: %s", strerror(errno));
            return EDN_CONNECT_INFO_INIT;
        }
        info.dst_ip = inet_ntop(AF_INET6, &addr.sin6_addr, nullptr, 0);
        info.dst_port = ntohs(addr.sin6_port);
        struct sockaddr_in6 local_addr;
        socklen_t local_addrlen = sizeof(local_addr);
        if (getsockname(fd, (struct sockaddr *)&local_addr, &local_addrlen) == -1) {
            EDN_LOG_ERROR("getsockname error: %s", strerror(errno));
            return EDN_CONNECT_INFO_INIT;
        }
        info.src_ip = inet_ntop(AF_INET6, &local_addr.sin6_addr, nullptr, 0);
        info.src_port = ntohs(local_addr.sin6_port);
    } else {
        sockaddr_in addr;
        socklen_t addrlen = sizeof(addr);
        if (getpeername(fd, (struct sockaddr *)&addr, &addrlen) == -1) {
            EDN_LOG_ERROR("getpeername error: %s", strerror(errno));
            return EDN_CONNECT_INFO_INIT;
        }
        info.dst_ip = inet_ntoa(addr.sin_addr);
        info.dst_port = ntohs(addr.sin_port);
        struct sockaddr_in local_addr;
        socklen_t local_addrlen = sizeof(local_addr);
        if (getsockname(fd, (struct sockaddr *)&local_addr, &local_addrlen) == -1) {
            EDN_LOG_ERROR("getsockname error: %s", strerror(errno));
            return EDN_CONNECT_INFO_INIT;
        }
        info.src_ip = inet_ntoa(local_addr.sin_addr);
        info.src_port = ntohs(local_addr.sin_port);
    }
    return copy_connect_info(info);
}

bool IsIpV6(const std::string &ip)
{
    return ip.find(":") != std::string::npos;
}

} // namespace edn