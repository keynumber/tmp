PROJECT(client)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# 输出编译过程详细信息
set(CMAKE_VERBOSE_MAKEFILE on)

ADD_EXECUTABLE(client   ./client.cpp
                        ../server/net_complete_func.cpp)

set(CMAKE_CXX_FLAGS "-std=c++11 -pthread -g -Wall")

include_directories(..)

# 可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ./bin)
