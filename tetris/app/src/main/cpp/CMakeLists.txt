cmake_minimum_required(VERSION 3.22.1)
project(TetrisNDK)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(tetris SHARED
        jni_bridge.cpp
        game.cpp)

find_library(log-lib log)
target_link_libraries(tetris ${log-lib})
