cmake_minimum_required(VERSION 3.15)
project(Chatroom)

set(CMAKE_CXX_STANDARD 11)

add_executable(Chatroom main.cpp)
target_link_libraries(Chatroom mysqlclient)