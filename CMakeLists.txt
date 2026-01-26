cmake_minimum_required(VERSION 3.13)

project(sqlite-starter-cpp)

set(CMAKE_CXX_STANDARD 23)

include_directories(src)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

add_executable(sqlite ${SOURCE_FILES})