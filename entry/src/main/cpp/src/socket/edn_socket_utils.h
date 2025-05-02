/***
 * Copyright (c) 2023, EDN
 * All rights reserved.
 * created by candy
 * 2025/3/16
 * @brief socket utils
 */

#ifndef EDN_EDN_SOCKET_UTILS_H
#define EDN_EDN_SOCKET_UTILS_H

#include "edn_define.h"
#include "edn_log.h"
#include "edn.h"
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <sstream>

namespace edn {
/**
 * @brief Set socket to non-blocking mode
 * @param fd Socket file descriptor
 * @return 0 on success, -1 on error
 */
int SetNonBlocking(int fd);
/**
 * @brief Create a socket
 * @return Socket file descriptor on success, -1 on error
 */
int Socket(bool use_ipv6 = false);
/**
 *  @brief Connect to a socket
 *  @param fd Socket file descriptor
 *  @param addr Address to connect to
 *  @param addrlen Length of the address structure
 *  @return EDN_OK on success, else on error
 */
int ConnectV4(int fd, const struct sockaddr *addr, socklen_t addrlen);
/**
 * @brief Bind local ip&port to a socket
 * @param fd Socket file descriptor
 * @param addr Address to bind with
 * @param addrlen Length of the address structure
 * @return EDN_OK on success, else on error
 */
int Bind(int fd, const struct sockaddr *addr, socklen_t addrlen);
/**
 * @brief Bind local ip&port to a socket
 * @param fd Socket file descriptor
 * @param ip ipv4 Address to bind with
 * @param port Port number
 * @return EDN_OK on success, else on error
 */
int BindV4(int fd, const std::string &ip, int port = 0);
/**
 * @brief Bind local ip&port to a socket
 * @param fd Socket file descriptor
 * @param addr ipv6 Address to bind with
 * @param port Port number
 * @return EDN_OK on success, else on error
 */
int BindV6(int fd, const std::string &addr, int port = 0);
/**
 * @brief Bind local ip&port to a socket
 * @param fd Socket file descriptor
 * @param addr ip Address to bind with
 * @param port Port number
 * @return EDN_OK on success, else on error
 */
int Bind(int fd, const std::string &ip, int port);
/**
 * @brief Conect to a socket
 * @param addr IP address
 * @param port Port number
 * @return EDN_OK on success, else on error
 */
int ConnectV4(int fd, const std::string &addr, int port);
/**
 * * @brief Connect to a socket
 * * @param fd Socket file descriptor
 * * @param addr Address to connect to
 * * @param addrlen Length of the address structure
 * * @return EDN_OK on success, else on error
 */
int ConnectV6(int fd, const std::string &addr, int port);
/**
 * * @brief Connect to a socket
 * * @param fd Socket file descriptor
 * * @param addr Address to connect to
 * * @param addrlen Length of the address structure
 * * @return EDN_OK on success, else on error
 */
int ConnectV6(int fd, const struct sockaddr *addr, socklen_t addrlen);
/**
 * * @brief Connect to a socket
 * * @param fd Socket file descriptor
 * * @param ip Address to connect to
 * * @param port Port number
 * * @return EDN_OK on success, else on error
 */
int Connect(int fd, const std::string ip, int port);
/**
 * * @brief Close a socket
 * * @param fd Socket file descriptor
 * * @return EDN_OK on success, else on error
 */
int Close(int fd);
/**
 * * @brief 获取当前文件描述符上可读取的字节数
 * * @param fd 文件描述符
 * * @return 可读取的字节数或-1表示出错
 */
int GetReadableBytes(int fd);
/**
 * * @brief 获取当前默认网卡的ipv4地址
 */
std::string GetDefaultIpV4();
/**
 * * @brief 获取当前默认网卡的ipv6地址
 */
std::string GetDefaultIpV6();
/**
 * * @brief 获取当前已经握手成功的fd的连接信息;
 * * @param fd 文件描述符
 * * @param is_ipv6 是否ipv6
 * * @return 连接信息EdnConnectInfo,使用后需主动释放
 */
EdnConnectInfo GetRealConnectInfo(int fd, bool is_ipv6 = false);
/**
 * 判断当前ip是否是ipv6
 * @param ip 地址
 * @return true 是ipv6 false 不是
 */
bool IsIpV6(const std::string &ip);

}
#endif