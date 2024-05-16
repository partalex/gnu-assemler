cmake_minimum_required(VERSION 3.28)
project(emulator)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(emulator main.cpp
        include/emulator.hpp
        src/emulator.cpp
        include/program.hpp
        src/program.cpp)
