
cmake_minimum_required(VERSION 3.4)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-everything")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp -O2")

# for Clion's code inspection
FILE(GLOB FILES
        ./*.hpp
        ./*.cpp)

set(SSORTOMP_SRC select_sort_omp.cpp)
set(GEN_DATA_SRC generate_data.cpp)

add_executable(ssort_np ${SSORTOMP_SRC})
add_executable(ssort_npi ${SSORTOMP_SRC})
add_executable(ssort_p ${SSORTOMP_SRC})
add_executable(ssort_pi ${SSORTOMP_SRC})
add_executable(gendata ${GEN_DATA_SRC})

set_target_properties(ssort_npi PROPERTIES COMPILE_FLAGS "-DINNER_PARALLEL")
set_target_properties(ssort_p PROPERTIES COMPILE_FLAGS "-DPARALLEL")
set_target_properties(ssort_pi PROPERTIES COMPILE_FLAGS "-DPARALLEL -DINNER_PARALLEL")
