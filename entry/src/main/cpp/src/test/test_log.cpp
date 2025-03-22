
#include "edn_log.h"
#include <stdio.h>
using namespace edn;

int main() {
    edn_set_log_cb(printf);
    EDN_LOG_DEBUG("test log %d", 1);
    EDN_LOG_INFO("test log %d", 2);
    EDN_LOG_WARN("test log %d", 3);
    EDN_LOG_ERROR("test log %d", 4);
    return 0;
}