cmake_minimum_required(VERSION 2.8)
project(g09DNNTSBuilder)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-comments -fopenmp -std=c++11")
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if (NOT (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9))
    set(CMAKE_CXX_COMPILER "g++-5")
    endif ()
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_COMPILER "/usr/local/bin/g++-5")
    include_directories("/usr/local/Cellar")
else()
    message(FATAL_ERROR "${PROJECT_NAME} requires gcc >= 4.9")
endif()


include_directories("$src/utils")
include_directories("include")
include_directories("/opt/intel/composer_xe_2015.3.187/mkl/include")
#add_subdirectory("include/glm")

set(SOURCE_FILES src/main.cpp)
add_executable(g09DNNTSBuilder ${SOURCE_FILES} src/handlers/input.cpp src/handlers/internalcoordinate.cpp src/core/tsbuilder.cpp src/utils/micro_timer.cpp include/glm/glm.hpp src/utils/conservation.cpp src/utils/conservation.h src/handlers/atom_masses.h)

