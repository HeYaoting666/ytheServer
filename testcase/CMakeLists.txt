cmake_minimum_required(VERSION 3.16)

# 工程名
project(TestConfig)

# 递归查找 src 目录下所有源文件
file(GLOB_RECURSE SOURCES "../src/*.cpp")

add_executable(${PROJECT_NAME} test_config.cpp ${SOURCES})

find_package(Threads)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} tinyxml)