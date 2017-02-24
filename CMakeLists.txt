cmake_minimum_required(VERSION 3.1)
project(hidpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(TINYXML2 tinyxml2)
pkg_check_modules(LIBUDEV libudev)

add_subdirectory(src/libhidpp)
add_subdirectory(src/tools)
add_subdirectory(doc/libhidpp)

