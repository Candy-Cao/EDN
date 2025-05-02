#include "edn_log.h"
#include "edn_utils.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace edn {
EdnLogger* EdnLogger::GetInstanse() {
    static EdnLogger logger;
    return &logger;
}

void EdnLogger::Log(int level, const char* file, int line, const char* func, const char *format, ...) {
    if (log_cb && level >= this->level) {
        char message[MAX_LOG_LEN] = {0};
        char formatted_message[MAX_LOG_LEN] = {0};
        va_list args;
        va_start(args, format);
        vsnprintf(message, sizeof(message), format, args);
        va_end(args);

        snprintf(formatted_message, sizeof(formatted_message), "[%s][%s:%d][%s][%s] %s\n",EdnUtils::GetTimeStamp().c_str(), file, line, func, GetLevelStr(level), message);
        log_cb("%s", formatted_message);
    }
}

const char* EdnLogger::GetLevelStr(int level) {
    switch (level) {
        case EDN_LOG_DEBUG:
            return "DEBUG";
        case EDN_LOG_INFO:
            return "INFO";
        case EDN_LOG_WARN:
            return "WARN";
        case EDN_LOG_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

// 异步信号安全的格式化输出函数
int EdnLogger::Print(const char* format, ...) {
    char buffer[MAX_LOG_LEN];  // 缓冲区大小可以根据需要调整
    va_list args;

    // 使用 vsnprintf 格式化字符串
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // 如果格式化成功，使用 write 输出
    if (len > 0 && len < (int)sizeof(buffer)) {
        write(STDOUT_FILENO, buffer, len);
        return len;  // 返回输出的字符数
    } else {
        const char* error_msg = "Error: Buffer overflow or formatting failed!\n";
        write(STDOUT_FILENO, error_msg, strlen(error_msg));
        return -1;  // 返回错误码
    }
}
}
