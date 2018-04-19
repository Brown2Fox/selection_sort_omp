
cmake_minimum_required(VERSION 3.4)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-everything")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-everything -std=c++11")


# for Clion's code inspection
FILE(GLOB FILES
        ./*.hpp
        ./*.cpp)

set(SSORTOMP_SRC select_sort_omp.cpp ${FILES})

add_executable(ssort ${SSORTOMP_SRC})