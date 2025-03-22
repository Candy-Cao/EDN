#include "edn.h"
#include "edn_log.h"

using namespace edn;

void edn_run() {}

int32_t edn_connect(EdnConnetInfo info, int timeout, pf_opt_complete_cb cb) {
    return EDN_OK;
}

EDN_CODE edn_send_data(int32_t connect_id, const char *buffer, int len, pf_opt_complete_cb cb) {
    return EDN_OK;
}

EDN_CODE edn_set_opt(int32_t connect_id, EDN_OPT_TYPE type, ...) {
    return EDN_OK;
}

EDN_CODE edn_close(int32_t connect_id) {
    return EDN_OK;
}

void edn_set_log_cb(pf_log_cb cb) {
    EdnLogger::GetInstanse()->SetLogCb(cb);
}
