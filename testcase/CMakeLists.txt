cmake_minimum_required(VERSION 3.16)

# 工程名
project(ytheSever)

# 递归查找 src 目录下所有源文件
file(GLOB_RECURSE SOURCES "../src/*.cpp")

# 设置 protobuf 源文件路径
set(PROTO_SRCS "../pb/compute.pb.cc")

add_executable(server test_server.cpp ${SOURCES} ${PROTO_SRCS})
add_executable(client test_client.cpp ${SOURCES} ${PROTO_SRCS})

find_package(Threads)
target_link_libraries(server ${CMAKE_THREAD_LIBS_INIT} tinyxml protobuf)
target_link_libraries(client ${CMAKE_THREAD_LIBS_INIT} tinyxml protobuf)

# add_executable(client test_client.cpp )