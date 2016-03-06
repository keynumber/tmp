project(test)
cmake_minimum_required(VERSION 2.6)

add_executable(test_io_wrapper  test_wrapper.cpp
                                ../common/wrapper.cpp)

add_executable(test_shift_writer    test_shift_writer.cpp
                                    ../common/shift_writer.cpp
                                    ../common/wrapper.cpp)

add_executable(test_logger  test_logger.cpp
                            ../common/logger.cpp
                            ../common/shift_writer.cpp
                            ../common/wrapper.cpp)

add_executable(test_fix_queue   test_fix_queue.cpp)

add_executable(test_event_notifier  test_event_notifier.cpp
                                    ../common/event_notifier.cpp
                                    ../common/wrapper.cpp)

add_executable(test_expire_queue test_expire_queue.cpp
                                 ../common/data_struct/expire_queue.cpp)

add_executable(test_fix_arry test_fix_array.cpp)

add_executable(test_rc_buf test_rc_buf.cpp)

add_executable(test_performace   test_performance.cpp
                                 ../common/performance_test.cpp)

add_executable(test_pool_list test_pool_list.cpp)

# 输出编译过程详细信息
set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_CXX_FLAGS "-std=c++11 -g")
set(CMAKE_EXE_LINKER_FLAGS "-pthread")

include_directories(..)

# 可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ./bin)
