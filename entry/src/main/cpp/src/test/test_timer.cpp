/***
 * @file test_timer.cpp
 * @brief 测试定时器
 * @author candy
 * @date 2025/3/16
 * @version 1.0
 */

#include "edn_log.h"
#include "edn_signal.h"
#include "edn_timer.h"
#include "edn_context.h"
#include "singleton.h"
#include <csignal>
#include <iostream>

using namespace edn;
int main() {
    auto ctx = Singleton<EdnContext>::getInstance();
    auto timer = std::make_shared<EdnTimer>(2000, true, []{EDN_LOG_INFO("timer trigger ***********");});
    auto sigev = std::make_shared<EdnSignal>(SIGINT, true, []{EDN_LOG_INFO("sigint trigger ***********");});
    ctx->AddEvent(sigev);
    ctx->AddEvent(timer);
    ctx->Run();
    return 0;
}