cmake_minimum_required(VERSION 2.8)

project(rusql)

include_directories("${PROJECT_BINARY_DIR}")

file(GLOB_RECURSE RUSQL_HEADERS src/*.hpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

# TODO install lines

add_subdirectory(tests)
enable_testing()
add_test(compile tests/compile)
add_test(connect ${CMAKE_SOURCE_DIR}/tests/runtest.pl connect)
add_test(connectfail ${CMAKE_SOURCE_DIR}/tests/runtest.pl connectfail)
