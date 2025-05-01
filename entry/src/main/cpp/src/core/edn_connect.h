/**
 * @file edn_connect.h
 * @brief 连接事件  
 * @details 处理连接事件的类，继承自EdnIOEvent类, 使用Connect之后在使用此类
 * @author  candy
 * @date 2025/3/16
 * @version 1.0
 */


#ifndef EDN_EDN_CONNECT_H
#define EDN_EDN_CONNECT_H
#include "edn_io_event.h"
#include "edn_buffer.h"
#include "edn_define.h"


namespace edn {


typedef std::function<int(const char*, int)> EdnHandleDataCallback;

typedef std::function<int(const char*, int)> EdnMessageEndCallback;

class EdnConnect: public EdnIOEvent {
public:
    EdnConnect(const std::string& ip, int port);
    ~EdnConnect() = default;

    /**
     * * @brief 异步发数据
     */
    int SendData(const char* data, size_t len, EdnAsyncOptCallback cb = nullptr);

    /**
     * * @brief 关闭连接
     */
    int Close();

    /**
     * * @brief 设置数据处理回调
     */
    int SetHandleDataCallback(EdnHandleDataCallback cb);

    /**
     * * @brief 设置消息结束回调
     */
    int SetMessageEndCallback(EdnMessageEndCallback cb);

private:
    static EdnIOEventCallback on_conn;
    static EdnIOEventCallback on_read;
    static EdnIOEventCallback on_write;
    static EdnIOEventCallback on_error;
    static EdnIOEventCallback on_close;

    std::shared_ptr<EdnBuffer> input_buffer_;        //输入缓冲区，用于读取数据
    std::shared_ptr<EdnBuffer> output_buffer_;       //输出缓冲区，用于发送数据

};

}


#endif //EDN_EDN_CONNECT_H