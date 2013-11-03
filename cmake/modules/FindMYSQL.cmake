# - Find MySQL
# Find the MySQL includes and client library
# This module defines
#  MYSQL_INCLUDE_DIR, where to find mysql.h
#  MYSQL_LIBRARIES, the libraries needed to use MySQL.
#  MYSQL_FOUND, If false, do not try to use MySQL.
#  MYSQLd_LIBRARIES, the libraries needed to use MySQL Embedded.
#  MYSQLd_FOUND, If false, do not try to use MySQL Embedded.
#
# Copyright (c) 2006, Jaroslaw Staniek, <js@iidea.pl>
# Updated by Sjors Gielen <mysql@sjor.sg>
# * Support for MySQL Embedded added
# * Hard-coded support for Windows removed
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

SET(MYSQL_CONFIG_PREFER_PATH "$ENV{MYSQL_HOME}/bin" CACHE FILEPATH
  "preferred path to MySQL (mysql_config)")

FIND_PROGRAM(MYSQL_CONFIG mysql_config
  ${MYSQL_CONFIG_PREFER_PATH}
  /usr/local/mysql/bin/
  /usr/local/bin/
  /usr/bin/
)

IF (MYSQL_CONFIG)
	EXEC_PROGRAM(${MYSQL_CONFIG}
		ARGS --include
		OUTPUT_VARIABLE MY_TMP)
	string (REGEX REPLACE "-I([^ ]*)( .*)?" "\\1" MY_TMP "${MY_TMP}")
	SET(MYSQL_INCLUDE_DIR ${MY_TMP} CACHE FILEPATH INTERNAL)

	EXEC_PROGRAM(${MYSQL_CONFIG}
		ARGS --libs_r
		OUTPUT_VARIABLE MY_TMP)

	string(REGEX MATCHALL "(^| )-l[^ ]*" MYSQL_LIB_LIST "${MY_TMP}")
	foreach(LIB ${MYSQL_LIB_LIST})
		string(REGEX REPLACE "[ ]*-l([^ ]*)" "\\1" LIB "${LIB}")
		list(APPEND MYSQL_LIBRARIES "${LIB}")
		#message("[DEBUG] MYSQL ADD_LIBRARIES : ${MYSQL_LIBRARIES}")
	endforeach(LIB ${MYSQL_LIB_LIST})

	set(MYSQL_ADD_LIBRARIES_PATH "")
	string(REGEX MATCHALL "(^| )-L[^ ]*" MYSQL_LIBDIR_LIST "${MY_TMP}")
	foreach(LIB ${MYSQL_LIBDIR_LIST})
		string(REGEX REPLACE "[ ]*-L([^ ]*)" "\\1" LIB "${LIB}")
		list(APPEND MYSQL_LIBRARIES_PATH "${LIB}")
		#message("[DEBUG] MYSQL ADD_LIBRARIES_PATH : ${MYSQL_LIBRARIES_PATH}")
	endforeach(LIB ${MYSQL_LIBDIR_LIST})

	EXEC_PROGRAM(${MYSQL_CONFIG}
		ARGS --libmysqld-libs
		OUTPUT_VARIABLE MY_TMP)

	string(REGEX MATCHALL "(^| )-l[^ ]*" MYSQL_LIB_LIST "${MY_TMP}")
	foreach(LIB ${MYSQL_LIB_LIST})
		string(REGEX REPLACE "[ ]*-l([^ ]*)" "\\1" LIB "${LIB}")
		list(APPEND MYSQLd_LIBRARIES "${LIB}")
		#message("[DEBUG] MYSQLd ADD_LIBRARIES : ${MYSQLd_LIBRARIES}")
	endforeach(LIB ${MYSQL_LIB_LIST})

	set(MYSQLd_LIBRARIES_PATH "")
	string(REGEX MATCHALL "(^| )-L[^ ]*" MYSQL_LIBDIR_LIST "${MY_TMP}")
	foreach(LIB ${MYSQL_LIBDIR_LIST})
		string(REGEX REPLACE "[ ]*-L([^ ]*)" "\\1" LIB "${LIB}")
		list(APPEND MYSQLd_LIBRARIES_PATH "${LIB}")
		#message("[DEBUG] MYSQLd ADD_LIBRARIES_PATH : ${MYSQLd_LIBRARIES_PATH}")
	endforeach(LIB ${MYSQL_LIBDIR_LIST})
ENDIF(MYSQL_CONFIG)

if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
   set(MYSQL_FOUND TRUE CACHE INTERNAL "MySQL found")
   message(STATUS "Found MySQL: ${MYSQL_INCLUDE_DIR}, ${MYSQL_LIBRARIES}")
else(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
   set(MYSQL_FOUND FALSE CACHE INTERNAL "MySQL found")
   message(STATUS "MySQL not found.")
endif(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)

if(MYSQL_INCLUDE_DIR AND MYSQLd_LIBRARIES)
   set(MYSQLd_FOUND TRUE CACHE INTERNAL "MySQL Embedded found")
   message(STATUS "Found MySQLd: ${MYSQL_INCLUDE_DIR}, ${MYSQLd_LIBRARIES}")
else(MYSQL_INCLUDE_DIR AND MYSQLd_LIBRARIES)
   set(MYSQLd_FOUND FALSE CACHE INTERNAL "MySQL Embedded found")
   message(STATUS "MySQLd not found.")
endif(MYSQL_INCLUDE_DIR AND MYSQLd_LIBRARIES)

mark_as_advanced(MYSQL_INCLUDE_DIR MYSQL_LIBRARIES MYSQLd_LIBRARIES)
