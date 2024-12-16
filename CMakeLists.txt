project(tiger CXX C)
cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/generated/lex.yy.c
    COMMAND flex -o ${CMAKE_CURRENT_SOURCE_DIR}/generated/lex.yy.c ${CMAKE_CURRENT_SOURCE_DIR}/flex.l
    DEPENDS flex.l
)

add_executable(
    tiger
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/lex.yy.c
    src/main.cpp
)