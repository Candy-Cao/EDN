//
// Created on 2025/3/23.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "edn_thread_pool.h"
#include <memory>
#include <iostream>
#include <unistd.h>

using namespace edn;

int main() {
    EdnThreadPoolPtr p = std::make_shared<EdnThreadPool>(2);
    for (int i = 0; i < 10; i++) {
        p->enqueue([=] {
            std::cout << "hello world " << i << std::endl;
        });
    }
    while (1) {
        sleep(1);
    }
    
}