cmake_minimum_required(VERSION 3.28)
project(linker)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(linker main.cpp
        include/linker.h
        src/linker.cpp
        include/loader_script_file.h
        src/loader_script_file.cpp
        include/object_file.h
        src/object_file.cpp
)
