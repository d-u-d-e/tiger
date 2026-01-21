cmake_minimum_required(VERSION 3.20)
project(tiger CXX C)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add dependencies
include(cmake/CPM.cmake)

function(configure_optional_define VAR DEFAULT)
    # If user did not pass -DVAR=...
    if(NOT DEFINED ${VAR})
        set(${VAR} "${DEFAULT}" PARENT_SCOPE)
    endif()
endfunction()

# All variables must have a default, otherwise autoconf.hpp will contain empty macros
configure_optional_define(CONFIG_TARGET_x86_64 0)
configure_optional_define(CONFIG_WITH_GRAPHVIZ 0)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/autoconf.hpp.in 
  ${CMAKE_CURRENT_BINARY_DIR}/generated/autoconf.hpp @ONLY)

set(sources_common
    src/string_table.cpp
    src/lexer/lex.cpp
    src/lexer/token.cpp
    src/parser/parser.cpp
    src/semant/types.cpp
    src/ir/tree.cpp
)

# Change this to include more targets
set(sources_target_specific)
if (CONFIG_TARGET_x86_64)
    list(APPEND sources_target_specific 
      src/arch/x86_64/generator.cpp
      src/arch/x86_64/frame.cpp
      )
endif()

add_executable(
    tigerc
    src/compiler.cpp
    src/semant/escape.cpp
    src/assem.cpp
    src/main.cpp
    src/ir/canon.cpp
    src/flow.cpp
    src/liveness.cpp
    src/register_allocator.cpp
    ${sources_common}
    ${sources_target_specific}
)

# link libraries
set(libraries "")
if (CONFIG_WITH_GRAPHVIZ)
    message(STATUS "Building with Graphviz support")
    list(APPEND libraries cgraph gvc)
endif()

target_include_directories(tigerc PRIVATE include ${CMAKE_CURRENT_BINARY_DIR})
target_compile_options(tigerc PRIVATE -Werror -Wall -Wextra -pedantic -g -O0)
target_link_libraries(tigerc ${libraries})

# build the runtime object
set(RUNTIME_SRC ${CMAKE_CURRENT_SOURCE_DIR}/lib/runtime.c)
set(LIB_DIR ${CMAKE_BINARY_DIR}/lib)
set(RUNTIME_OBJ ${LIB_DIR}/runtime.o)

file(MAKE_DIRECTORY ${LIB_DIR})
add_custom_command(
    OUTPUT ${RUNTIME_OBJ}
    COMMAND ${CMAKE_C_COMPILER} -c ${CMAKE_C_FLAGS} ${RUNTIME_SRC} -o ${RUNTIME_OBJ}
    DEPENDS ${RUNTIME_SRC}
    COMMENT "Building runtime.o into ${LIB_DIR}"
)

add_custom_target(runtime ALL DEPENDS ${RUNTIME_OBJ})
add_dependencies(tigerc runtime)

set_target_properties(tigerc PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set(BUILD_TEST_RUNNER ON)
# Optionally build the test runner
if(BUILD_TEST_RUNNER)
    CPMAddPackage("https://github.com/doctest/doctest.git@2.4.11")
    add_executable(test-runner
        tests/main.cpp
        tests/lexer/test_lexer.cpp
        tests/parser/test_parser.cpp
        tests/semant/test_env.cpp
        tests/semant/test_analyzer.cpp
        ${sources_common}
    )

    target_link_libraries(test-runner doctest::doctest)
    target_include_directories(test-runner PRIVATE include ${CMAKE_CURRENT_BINARY_DIR})
    target_compile_options(test-runner PRIVATE -Werror -Wall -Wextra -Wno-maybe-uninitialized -g -O2)
    
endif(BUILD_TEST_RUNNER)


# Install compiler executable
install(TARGETS tigerc RUNTIME DESTINATION bin)
# Install runtime.o into lib/
install(FILES ${RUNTIME_OBJ} DESTINATION lib)
# Install runtime.o into lib/
install(FILES tools/driver.sh 
    DESTINATION tools
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                GROUP_EXECUTE GROUP_READ
                WORLD_EXECUTE WORLD_READ
)