cmake_minimum_required(VERSION 3.16)
project(kShortestPaths)

set(CMAKE_CXX_STANDARD 20)

add_executable(kShortestPaths main.cpp graph.cpp kbest.cpp)