cmake_minimum_required(VERSION 3.20)

project(copper)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

add_subdirectory(dep)
add_subdirectory(src)
add_subdirectory(samples)

