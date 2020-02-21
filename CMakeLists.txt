cmake_minimum_required(VERSION 3.15)
project(Chatroom)

set(CMAKE_CXX_STANDARD 11)

add_executable(Chatroom Client.cpp Server.cpp Common.cpp main.cpp Login.cpp Friendlist.cpp Talk.cpp)

target_link_libraries(Chatroom mysqlclient)

target_link_libraries(Chatroom crypt)