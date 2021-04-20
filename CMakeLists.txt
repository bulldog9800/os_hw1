cmake_minimum_required(VERSION 3.19)
project(os_hw1)

set(CMAKE_CXX_STANDARD 11)

add_executable(os_hw1 main.cpp Commands.h Commands.cpp smash.cpp signals.h signals.cpp)