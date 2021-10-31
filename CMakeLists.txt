cmake_minimum_required(VERSION 3.21)

project(epoll_demo)
INCLUDE_DIRECTORIES(./include)
SET(CMAKE_CXX_FLAGS "-g -std=c++11 -lpthread")
file(GLOB lib impl/*.cpp)
set(src    ${lib} main.cpp  )
message(" src: " ${src})

ADD_EXECUTABLE(server ${src} )
