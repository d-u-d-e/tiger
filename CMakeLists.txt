cmake_minimum_required(VERSION 3.8)
project(tiger CXX C)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add dependencies
include(cmake/CPM.cmake)

add_executable(
    tiger
    src/main.cpp
    src/lexer/lex.cpp
    src/lexer/token.cpp
)
target_include_directories(tiger PRIVATE include)

#set(BUILD_TEST_RUNNER ON)
# Optionally build the test runner
if(BUILD_TEST_RUNNER)

    CPMAddPackage("https://github.com/doctest/doctest.git@2.4.11")
    add_executable(test-runner
    tests/main.cpp
    tests/lexer/test_lexer.cpp
    src/lexer/lex.cpp
    src/lexer/token.cpp
    )
    target_link_libraries(test-runner doctest::doctest)
    target_include_directories(test-runner PRIVATE include)
endif(BUILD_TEST_RUNNER)