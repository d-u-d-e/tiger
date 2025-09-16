cmake_minimum_required(VERSION 3.8)
project(tiger CXX C)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add dependencies
include(cmake/CPM.cmake)

set(sources_common

    src/lexer/lex.cpp
    src/lexer/token.cpp
    src/parser/parser.cpp
    src/symbol.cpp
    src/seman/types.cpp
    src/seman/env.cpp
    src/seman/analyzer.cpp
    src/seman/escape.cpp
    src/ir/translator.cpp
)

add_executable(
    tigerc
    src/main.cpp
    src/ir/canon.cpp
    src/ir/tree.cpp
    src/codegen/arch/isel.cpp
    src/flow.cpp
    ${sources_common}
)
target_include_directories(tigerc PRIVATE include)
target_compile_options(tigerc PRIVATE -Werror -Wall -Wextra -pedantic -g -O0)
target_link_libraries(tigerc cgraph gvc)

set(BUILD_TEST_RUNNER ON)
# Optionally build the test runner
if(BUILD_TEST_RUNNER)
    CPMAddPackage("https://github.com/doctest/doctest.git@2.4.11")
    add_executable(test-runner
        tests/main.cpp
        tests/lexer/test_lexer.cpp
        tests/parser/test_parser.cpp
        tests/seman/test_env.cpp
        tests/seman/test_analyzer.cpp
        ${sources_common}
    )

    target_link_libraries(test-runner doctest::doctest)
    target_include_directories(test-runner PRIVATE include)
    target_compile_options(test-runner PRIVATE -Werror -Wall -Wextra -Wno-maybe-uninitialized -O2)
    
endif(BUILD_TEST_RUNNER)