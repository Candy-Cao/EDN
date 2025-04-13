project(edn)

cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_TOOLCHAIN_FILE "D:\\SoftWare\\DevEco Studio\\sdk\\default\\openharmony\\native\\build\\cmake\\ohos.toolchain.cmake")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/event
    ${CMAKE_CURRENT_SOURCE_DIR}/hash
    ${CMAKE_CURRENT_SOURCE_DIR}/socket
    ${CMAKE_CURRENT_SOURCE_DIR}/buffer
    ${CMAKE_CURRENT_SOURCE_DIR}/test
)   

file (GLOB SOURCES "*.cpp" "*/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX "/test/")

set(TEST_LOG_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test/test_log.cpp)

set(TEST_SIG_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test/test_signal.cpp)

set(TEST_THREAD_POOL_SRC ${CMAKE_CURRENT_SOURCE_DIR}/test/test_thread_pool.cpp)

add_library(edn ${SOURCES})

add_executable(test_log ${TEST_LOG_SRC})
add_executable(test_sig ${TEST_SIG_SRC})
add_executable(test_thread_pool ${TEST_THREAD_POOL_SRC})
add_executable(test_timer ${CMAKE_CURRENT_SOURCE_DIR}/test/test_timer.cpp)
add_executable(test_io ${CMAKE_CURRENT_SOURCE_DIR}/test/test_io_event.cpp)
target_link_libraries(test_log edn)

target_link_libraries(test_sig edn)

target_link_libraries(test_thread_pool edn)

target_link_libraries(test_timer edn)

target_link_libraries(test_io edn)

