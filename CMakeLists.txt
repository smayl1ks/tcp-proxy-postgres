cmake_minimum_required(VERSION 3.8.1)

project(proxy_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++17")

add_library(${PROJECT_NAME}
    src/proxy_server.cpp
    src/logger.cpp
    src/parser.cpp)

add_executable(proxy src/main.cpp)

target_link_libraries(proxy ${PROJECT_NAME})
