/***
 * Copyright (c) 2023, EDN
 * All rights reserved.
 * created by candy
 * 2025/3/16
 * @brief socket utils
 */

#ifndef EDN_EDN_SOCKET_UTILS_H
#define EDN_EDN_SOCKET_UTILS_H

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
}
#endif