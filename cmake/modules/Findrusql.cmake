# - Try to find rusql
# Once done this will define
#  rusql_FOUND			- system has rusql
#  rusql_INCLUDE_DIRS		- include directories for rusql
#  rusql_LIBRARIES		- libraries for rusql

message(STATUS "Checking for package 'rusql'")

set(rusql_FOUND FALSE)

set(rusql_EXPECTED_PATHS /usr/local/include /usr/include)
find_path(rusql_INCLUDE_DIRS rusql/connection.hpp ${rusql_EXPECTED_PATHS})

find_package(MysqlCppConn REQUIRED)

set(rusql_INCLUDE_DIRS ${rusql_INCLUDE_DIRS} ${MysqlCppConn_INCLUDE_DIRS})
set(rusql_LIBRARIES ${MysqlCppConn_LIBRARIES})


if(rusql_INCLUDE_DIRS AND rusql_LIBRARIES)
	set(rusql_FOUND TRUE)
endif()

mark_as_advanced(rusql_INCLUDE_DIRS rusql_LIBRARIES rusql_FOUND)
