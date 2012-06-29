# - Try to find MysqlCppConn
# Once done this will define
#  MYSQLCPPCONN_FOUND		- system has MysqlCppConn
#  MYSQLCPPCONN_INCLUDE_DIRS	- include directories for MysqlCppConn
#  MYSQLCPPCONN_LIBRARIES	- libraries for MysqlCppConn

message(STATUS "Checking for package 'MysqlCppConn'")

set(MYSQLCPPCONN_FOUND FALSE)

find_path(MYSQLCPPCONN_INCLUDE_DIRS mysql_connection.h
	/usr/include
)

find_library(MYSQLCPPCONN_LIBRARIES
	NAMES mysqlcppconn
)

if(MYSQLCPPCONN_INCLUDE_DIRS AND MYSQLCPPCONN_LIBRARIES)
	set(MYSQLCPPCONN_FOUND TRUE)
else()
	message(FATAL_ERROR "Could not find MysqlCppConn library")
endif(MYSQLCPPCONN_INCLUDE_DIRS AND MYSQLCPPCONN_LIBRARIES)

mark_as_advanced(MYSQLCPPCONN_INCLUDE_DIRS MYSQLCPPCONN_LIBRARIES MYSQLCPPCONN_FOUND)
