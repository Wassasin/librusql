cmake_minimum_required(VERSION 2.8)

project(rusql)

include_directories("${PROJECT_BINARY_DIR}")

file(GLOB_RECURSE RUSQL_HEADERS src/*.hpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

install(FILES ${RUSQL_HEADERS} DESTINATION include/rusql)
install(FILES
	cmake/modules/FindMysqlCppConn.cmake
	cmake/modules/Findrusql.cmake

	DESTINATION share/cmake-2.8/Modules
)

add_subdirectory(tests)
enable_testing()
add_test(compile tests/compile)
add_test(connect ${CMAKE_SOURCE_DIR}/tests/runtest.pl connect)
add_test(connectfail ${CMAKE_SOURCE_DIR}/tests/runtest.pl connectfail)
