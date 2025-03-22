#include "edn_log.h"
#include <cstdarg>
#include <cstdio>


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

            snprintf(formatted_message, sizeof(formatted_message), "[%s:%d][%s][%s] %s\n", file, line, func, GetLevelStr(level), message);
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
}
