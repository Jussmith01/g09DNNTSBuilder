cmake_minimum_required(VERSION 2.8)
project(g09DNNTSBuilder)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-comments -fopenmp -std=c++11")
set(CMAKE_CXX_COMPILER "/usr/bin/g++-4.9")
include_directories("$src/utils")
include_directories("include")
add_subdirectory("include/glm")


set(SOURCE_FILES src/main.cpp)
add_executable(g09MLData ${SOURCE_FILES} src/handlers/input.cpp src/handlers/internalcoordinate.cpp include/glm/glm.hpp)