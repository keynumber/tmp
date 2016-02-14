PROJECT(server)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# 输出编译过程详细信息
set(CMAKE_VERBOSE_MAKEFILE on)

ADD_EXECUTABLE(server   ./main.cpp

                        ./acceptor.cpp
                        ./io_handler.cpp
                        ./worker.cpp
                        ./controller.cpp
                        ./message_center.cpp
                        ./default_net_complete_func.cpp

                        ../common/logger.cpp
                        ../common/shift_writer.cpp
                        ../common/wrapper.cpp
                        ../common/event_notifier.cpp
                        ../common/data_struct/expire_queue.cpp
                        ../common/util.cpp
                        ../common/poller.cpp)

set(CMAKE_CXX_FLAGS "-std=c++11 -pthread -g -Wall")

include_directories(..)

# 可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ./bin)
