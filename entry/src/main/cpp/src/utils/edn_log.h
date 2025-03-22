#ifndef EDN_LOG_H
#define EDN_LOG_H

#include "edn.h"

namespace edn {

#define MAX_LOG_LEN 1024
typedef enum EDN_LOG_LEVEL {
    EDN_LOG_DEBUG,
    EDN_LOG_INFO,
    EDN_LOG_WARN,
    EDN_LOG_ERROR
} EDN_LOG_LEVEL;

class EdnLogger {
public:
    EdnLogger() {
        log_cb = nullptr;
    }
    void SetLogCb(pf_log_cb cb) {
        log_cb = cb;
    }
    void SetLogLevel(int level) {
        this->level = level;
    }
    void Log(int level, const char* file, int line, const char* func, const char* format, ...);

    const char* GetLevelStr(int level);

    static EdnLogger* GetInstanse();

private:
    
    pf_log_cb log_cb;
    int level = EDN_LOG_DEBUG;
};

#define EDN_LOG(level, format, ...) edn::EdnLogger::GetInstanse()->Log(level, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define EDN_LOG_DEBUG(format, ...) EDN_LOG(EDN_LOG_DEBUG, format, ##__VA_ARGS__)
#define EDN_LOG_INFO(format, ...) EDN_LOG(EDN_LOG_INFO, format, ##__VA_ARGS__)
#define EDN_LOG_WARN(format, ...) EDN_LOG(EDN_LOG_WARN, format, ##__VA_ARGS__)
#define EDN_LOG_ERROR(format, ...) EDN_LOG(EDN_LOG_ERROR, format, ##__VA_ARGS__)

}

#endif // EDN_LOG_H