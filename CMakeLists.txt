cmake_minimum_required(VERSION 3.16)
project(kShortestPaths)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_library(kShortestPaths source/graph.cpp source/kbest.cpp)

add_executable(kShortestPaths_example source/main.cpp )
target_link_libraries(kShortestPaths_example kShortestPaths)