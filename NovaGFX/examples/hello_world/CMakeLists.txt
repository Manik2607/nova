cmake_minimum_required(VERSION 3.20)
project(hello_world)

add_executable(hello_world main.cpp)
target_link_libraries(hello_world PRIVATE nova)
