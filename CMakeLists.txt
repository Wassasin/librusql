cmake_minimum_required(VERSION 2.8)

project(rusql)

include_directories("${PROJECT_BINARY_DIR}")

file(GLOB_RECURSE RUSQL_HEADERS src/*.hpp)
file(GLOB_RECURSE RUSQL_SOURCES src/*.cpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

find_package(MysqlCppConn REQUIRED)
include_directories(${MysqlCppConn_INCLUDE_DIRS})
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_library(rusql ${RUSQL_SOURCES})
target_link_libraries(rusql ${MysqlCppConn_LIBRARIES} ${Boost_LIBRARIES})

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
