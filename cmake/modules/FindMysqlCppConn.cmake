# - Try to find MysqlCppConn
# Once done this will define
#  MysqlCppConn_FOUND		- system has MysqlCppConn
#  MysqlCppConn_INCLUDE_DIRS	- include directories for MysqlCppConn
#  MysqlCppConn_LIBRARIES	- libraries for MysqlCppConn

message(STATUS "Checking for package 'MysqlCppConn'")

set(MysqlCppConn_FOUND FALSE)

find_path(MysqlCppConn_INCLUDE_DIRS mysql_connection.h
	/usr/include
)

find_library(MysqlCppConn_LIBRARIES
	NAMES mysqlcppconn
)

if(MysqlCppConn_INCLUDE_DIRS AND MysqlCppConn_LIBRARIES)
	set(MysqlCppConn_FOUND TRUE)
endif()

mark_as_advanced(MysqlCppConn_INCLUDE_DIRS MysqlCppConn_LIBRARIES MysqlCppConn_FOUND)
