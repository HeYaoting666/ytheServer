cmake_minimum_required(VERSION 3.16)

# 工程名
project(Test)

# 递归查找 src 目录下所有源文件
file(GLOB_RECURSE SOURCES "../src/*.cpp")

add_executable(server test_server.cpp ${SOURCES})
add_executable(client test_client.cpp ${SOURCES})

find_package(Threads)
target_link_libraries(server ${CMAKE_THREAD_LIBS_INIT} tinyxml)
target_link_libraries(client ${CMAKE_THREAD_LIBS_INIT} tinyxml)