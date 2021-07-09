cmake_minimum_required(VERSION 3.19)
project(ThreadPol)

set(CMAKE_CXX_STANDARD 17)

add_executable(ThreadPol main.cpp ThreadPool.h Task.h)