cmake_minimum_required(VERSION 3.20)
project(MyProject CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/source)
set(EXTERN_DIR ${CMAKE_SOURCE_DIR}/extern)
set(EXE_DIR ${CMAKE_SOURCE_DIR}/execute)

add_executable(hello
        ${EXE_DIR}/first.cpp
)

add_executable(hello_pixel
        ${EXE_DIR}/hello_pixel.cpp
)

add_executable(simple_sky
        ${EXE_DIR}/simple_sky_ray.cpp
)

add_executable(hello_sphere
        ${EXE_DIR}/hello_sphere.cpp
)
# add_executable(hello_sphere
#         #${EXE_DIR}/hello_sphere.cpp
# )
#find_package(assimp REQUIRED)
#find_package(spdlog REQUIRED)