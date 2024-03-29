cmake_minimum_required(VERSION 3.23)
project(ProjectX)

set(CMAKE_CXX_STANDARD 17)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# 配置OpenMP
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

include_directories(
        include
)

add_executable(
        rasterize
        src/rasterize.cpp
)
add_executable(
        ray_trace
        src/ray_trace.cpp
)
add_executable(
        matrix_test
        src/math_test.cpp
        include/Just/Core/Emitter.h)
