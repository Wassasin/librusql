cmake_minimum_required(VERSION 2.8)

project(rusql)

include_directories("${PROJECT_BINARY_DIR}")

file(GLOB_RECURSE RUSQL_HEADERS src/*.hpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

find_package(MysqlCppConn)
find_package(Boost COMPONENTS system REQUIRED)

set(RUSQL_INCLUDE_DIRS ${MYSQLCPPCONN_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
set(RUSQL_LIBRARIES ${MYSQLCPPCONN_LIBRARIES} ${Boost_SYSTEM_LIBRARY})
