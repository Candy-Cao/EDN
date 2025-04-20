
#ifndef __EDN_DEFINE_H__
#define __EDN_DEFINE_H__

#include <string>

namespace edn {

#define INVALID_FD -1
#define INVALID_SIG -1

#define DEFAULT_BUFFER_LEN 4 * 1024 * 20
#define DEFAULT_MIN_BUFFER_NODE_LEN 4096
#define DEFAULT_WRITE_HIGH_WATER_MARK 4096 * 16
#define DEFAULT_WRITE_LOW_WATER_MARK 4096
#define DEFAULT_READ_LOW_WATER_MARK 4096
#define DEFAULT_READ_HIGH_WATER_MARK 4096

struct EdnError {
    int edn_code;
    int sys_code;
    std::string errmsg;
};

}

#endif