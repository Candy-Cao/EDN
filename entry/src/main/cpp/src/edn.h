/*
MIT License

Copyright (c) 2025 Candy Cao

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#ifndef __NET_EDN_H__
#define __NET_EDN_H__

#include <cstdint>
#define EDN_VERSION 1.0
#define EDN_VERSION_MAJOR 1
#define EDN_VERSION_MINOR 0
#define EDN_VERSION_PATCH 0

#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#ifdef _WIN32
  #define EDN_API __declspec(dllexport)
#else
  #define EDN_API __attribute__((visibility("default")))
#endif


#define EDN_OPT_STRING 0
#define EDN_OPT_INT 1000
#define EDN_OPT_PVOID 2000
#define EDN_OPT(name, type, num) EDN_OPT##_TYPE_##name = EDN_OPT_##type + num

#ifdef __cplusplus
extern "C" {
#endif

typedef enum EDN_CODE {
    EDN_OK = 0,
    EDN_ERR_PARAMS_INVALID = -1,
    EDN_ERR_SOCK_ERROR = -2,
    EDN_ERR_TIMEOUT = -3,
    EDN_ERR_SYS_ERROR = -4,
    EDN_ERR_BUFFER_NOT_ENOUGH = -5,
} EDN_CODE;

typedef enum EDN_OPT_TYPE {
    EDN_OPT(HANDLE_DATA_CB, PVOID, 1),
    EDN_OPT(MESSAGE_END_CB, PVOID, 2),
} EDN_OPT_TYPE;

typedef void (*pf_opt_complete_cb)(int32_t error);

typedef int (*pf_log_cb)(const char* format, ...);

typedef struct EdnConnetInfo {
    const char* src_ip;
    uint32_t src_port;
    const char *dst_ip;
    uint32_t dst_port;
} EdnConnectInfo;

// event loop
EDN_API void edn_run();

//create a tcp connect ;return fd if success else EDN_CODE
EDN_API int32_t edn_connect(EdnConnetInfo info, int timeout, pf_opt_complete_cb cb);

//发送数据，操作完成回调函数只会保持一个；如果设置了输出缓冲区低水位线，
//则会在数据发送且缓冲区低于低水位线时触发回调函数；
EDN_API EDN_CODE edn_send_data(int32_t connect_id, const char *buffer, int len, pf_opt_complete_cb cb);

//异步数据处理函数, 返回值表示已经处理的数据的长度
typedef int32_t (*pf_handle_data_cb)(const char* buffer, int len);

//数据包结束标志回调,返回0表示消息包没有结束;否则返回预计的消息包的长度
typedef int32_t (*pf_message_package_end_cb)(const char* buffer, int len);

EDN_API EDN_CODE edn_set_opt(int32_t connect_id, EDN_OPT_TYPE type, ...);

EDN_API EDN_CODE edn_close(int32_t connect_id);

EDN_API void edn_set_log_cb(pf_log_cb cb);

#ifdef __cplusplus
}
#endif



#endif