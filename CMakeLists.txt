cmake_minimum_required(VERSION 3.8)
project(tiger CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
    tiger
    src/main.cpp
    src/lexer/lex.cpp
    src/lexer/token.cpp
)

target_include_directories(tiger PRIVATE include)