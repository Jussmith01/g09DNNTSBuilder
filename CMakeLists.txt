cmake_minimum_required(VERSION 2.8)
project(g09DNNTSBuilder)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-comments -fopenmp -std=c++11")
set(CMAKE_CXX_COMPILER "g++")
include_directories("$src/utils")
include_directories("include")
include_directories("/home/jsmith48/scratch/compilers/usr/include/c++/5.2.0")
#add_subdirectory("include/glm")

set(SOURCE_FILES src/main.cpp)
add_executable(bin/CmakeBuild/g09DNNTSBuilder ${SOURCE_FILES} src/handlers/input.cpp src/handlers/internalcoordinate.cpp src/core/tsbuilder.cpp src/utils/micro_timer.cpp include/glm/glm.hpp)

