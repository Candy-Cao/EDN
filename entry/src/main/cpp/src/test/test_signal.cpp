//
// Created on 2025/3/22.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "edn_log.h"
#include "edn_signal.h"
#include "edn_context.h"
#include "singleton.h"
#include <csignal>
#include <iostream>


using namespace edn;
using namespace std;

void sigint_handler(int sig) {
    cout << "sigint trigger..." << endl;
}

int main() {
//    auto ctx = Singleton<EdnContext>::getInstance();
//    auto sigev = std::make_shared<EdnSignal>(SIGINT, true, []{EDN_LOG_INFO("sigint trigger ***********");});
//    ctx->AddEvent(sigev);
//    ctx->Run();
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&(sa.sa_mask));
    sigaction(SIGINT, &sa, NULL);
    sleep(10);
    return 0;
}