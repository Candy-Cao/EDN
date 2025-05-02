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
#include "edn_hash_table.h"
#include "edn_timer.h"


namespace edn {


class EdnConnect: public EdnIOEvent {
public:
    EdnConnect(const std::string& ip, int port);

    EdnConnect(const EdnConnectInfo&, EdnAsyncOptCallback cb);
    EdnConnect(const EdnConnect&) = delete;
    EdnConnect& operator=(const EdnConnect&) = delete;
    EdnConnect(EdnConnect&&) = delete;
    EdnConnect& operator=(EdnConnect&&) = delete;
    ~EdnConnect();

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

    /**
     * * @brief 格式化连接信息
     */
    static std::string Format(const EdnConnectInfo &info);

    /**
     * * @brief 设置连接信息
     */
    void SetConnectInfo(const EdnConnectInfo &info);

    /**
     * * @brief 获取连接信息
     */
    EdnConnectInfo GetConnectInfo() const;

    /**
     * * @brief 设置连接超时定时器
     */
    void SetTimer(EdnTimerPtr timer);

    /**
     * * @brief 获取连接超时定时器
     */
    EdnTimerPtr GetTimer() const;

    /**
     * * @brief 规范化连接信息
     */
    static EdnConnectInfo ConnectInfoNomalize(const EdnConnectInfo &info);

public:

    static EdnHashTablePtr conn_table; //哈希表，用于存储连接信息和文件描述符的映射关系

private:
    static EdnIOEventCallback on_conn;
    static EdnIOEventCallback on_read;
    static EdnIOEventCallback on_write;
    static EdnIOEventCallback on_error;
    static EdnIOEventCallback on_close;

    std::shared_ptr<EdnBuffer> input_buffer_;        //输入缓冲区，用于读取数据
    std::shared_ptr<EdnBuffer> output_buffer_;       //输出缓冲区，用于发送数据

    EdnConnectInfo connect_info_; //连接信息

    EdnTimerPtr timer_; //定时器，用于连接超时处理
};

}


#endif //EDN_EDN_CONNECT_H