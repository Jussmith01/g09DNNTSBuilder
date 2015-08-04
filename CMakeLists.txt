cmake_minimum_required(VERSION 2.8)
project(g09DNNTSBuilder)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-comments -fopenmp -std=c++11")
include_directories("$(PROJECT_SOURCE_DIR)/src/utils")
include_directories("$(PROJECT_SOURCE_DIR)/include")


set(SOURCE_FILES src/main.cpp)
add_executable(g09MLData ${SOURCE_FILES} )