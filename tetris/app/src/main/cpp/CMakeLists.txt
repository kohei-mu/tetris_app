cmake_minimum_required(VERSION 3.22.1)
project(TetrisNDK)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(tetris_core STATIC game.cpp)
set_target_properties(tetris_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(tetris SHARED jni_bridge.cpp)

find_library(log-lib log)
target_link_libraries(tetris tetris_core ${log-lib})
