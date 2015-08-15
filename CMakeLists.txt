cmake_minimum_required(VERSION 2.8)
project(g09DNNTSBuilder)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-comments -fopenmp -std=c++11")
set(CMAKE_CXX_COMPILER "/usr/local/bin/g++-5")
include_directories("$src/utils")
include_directories("include")
include_directories("/usr/local/Cellar")
include_directories("/opt/intel/composer_xe_2015.3.187/mkl/include")
#add_subdirectory("include/glm")

set(SOURCE_FILES src/main.cpp)
add_executable(g09DNNTSBuilder ${SOURCE_FILES} src/handlers/input.cpp src/handlers/internalcoordinate.cpp src/core/tsbuilder.cpp src/utils/micro_timer.cpp include/glm/glm.hpp src/utils/conservation.cpp src/utils/conservation.h src/handlers/atom_masses.h)

