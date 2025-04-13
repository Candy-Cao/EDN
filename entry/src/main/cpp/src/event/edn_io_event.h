/**
 * @file edn_io_event.h
 * @brief IO事件
 * @author candy
 * @date 2025/3/16
 * @version 1.0
 */

#ifndef EDN_EDN_IO_EVENT_H
#define EDN_EDN_IO_EVENT_H
#include "edn_event.h"
#include <memory>
#include <functional>


namespace edn {

typedef std::function<int(void*)> EdnIOEventCallback;
/**
 * @brief IO事件
 * @details 处理IO事件的类，继承自EdnEvent类, 使用Connect之后在使用此类
 */
class EdnIOEvent: public EdnEvent {
public:
    EdnIOEvent(int fd, bool et = true);

    EdnIOEvent(int fd, EdnIOEventCallback conn_cb,
               EdnIOEventCallback read_cb,
               EdnIOEventCallback write_cb,
               EdnIOEventCallback error_cb,
               EdnIOEventCallback close_cb, 
               bool et = true);
    
    ~EdnIOEvent() ;

    int handler() override;

    void SetRealEvents(unsigned int events);
    /**
     * * @brief 将epoll事件转换为edn 事件类型
     */
    int ConvertEvents(unsigned int events);
    /**
     * edn IO事件类型
     */
    typedef enum {
        NONE = 0,
        CONNECTING = 1,
        CONNECTED = 2,
        READ = 3,
        WRITE = 4,
        ERROR = 5,
        CLOSE = 6,
    } ConnectStatus;

    void SetConnectStatus(ConnectStatus status) {
        connect_status_ = status;
    }

    void SetConnCb(EdnIOEventCallback conn_cb) {
        conn_cb_ = conn_cb;
    }
    void SetReadCb(EdnIOEventCallback read_cb) {
        read_cb_ = read_cb;
    }
    void SetWriteCb(EdnIOEventCallback write_cb) {
        write_cb_ = write_cb;
    }
    void SetErrorCb(EdnIOEventCallback error_cb) {
        error_cb_ = error_cb;
    }
    void SetCloseCb(EdnIOEventCallback close_cb) {
        close_cb_ = close_cb;
    }

private:
    static EdnIOEventCallback def_conn_cb;
    static EdnIOEventCallback def_read_cb;
    static EdnIOEventCallback def_write_cb;
    static EdnIOEventCallback def_error_cb;
    static EdnIOEventCallback def_close_cb;
    unsigned int real_events_ = 0;
    ConnectStatus connect_status_ = NONE;
    EdnIOEventCallback conn_cb_;
    EdnIOEventCallback read_cb_;    
    EdnIOEventCallback write_cb_;
    EdnIOEventCallback error_cb_;
    EdnIOEventCallback close_cb_;
    
};
typedef std::shared_ptr<EdnIOEvent> EdnIOEventPtr;

} // namespace edn




#endif //EDN_EDN_IO_EVENT_H