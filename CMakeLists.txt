cmake_minimum_required(VERSION 3.8)
project(LZ77)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES lz77.cpp lz77.h)
add_executable(LZ77 ${SOURCE_FILES})