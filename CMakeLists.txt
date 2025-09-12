cmake_minimum_required(VERSION 3.20)
project(MyProject CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/source)
set(EXTERN_DIR ${CMAKE_SOURCE_DIR}/extern)
set(EXE_DIR ${CMAKE_SOURCE_DIR}/execute)

include_directories(
    ${SOURCE_DIR}
    ${EXTERN_DIR}
    )  # EXE_DIR는 제거
find_package(spdlog REQUIRED)
add_executable(hello ${EXE_DIR}/first.cpp
        source/Scene.cpp
        source/Scene.hpp)
add_executable(hello_pixel ${EXE_DIR}/hello_pixel.cpp)
add_executable(simple_sky ${EXE_DIR}/simple_sky_ray.cpp)
add_executable(hello_sphere ${EXE_DIR}/hello_sphere.cpp)
add_executable(hello_world ${EXE_DIR}/hello_world.cpp)
add_executable(hello_diffuse ${EXE_DIR}/hello_diffuse.cpp)
target_link_libraries(hello_diffuse spdlog::spdlog)

add_executable(hello_metal ${EXE_DIR}/hello_metal.cpp)
target_link_libraries(hello_metal spdlog::spdlog)