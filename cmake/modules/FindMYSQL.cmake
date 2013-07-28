# - Find MySQL
#
# MYSQL_FOUND
# MYSQL_INCLUDE_DIRS
# MYSQL_LIBRARIES

include(LibFindMacros)

libfind_package(MYSQL zlib)

libfind_pkg_check_modules(MYSQL_PKGCONF mysql)

find_path(MYSQL_INCLUDE_DIR
	NAMES mysql.h
	PATHS
	${MYSQL_PKGCONF_INCLUDE_DIRS}
	/usr/local/include/mysql/
	/usr/include/mysql/
	/sw/include/mysql/
	/opt/local/include/mysql55/mysql
)

find_library(MYSQL_LIBRARY
	NAMES mysql mysqlclient
	PATHS ${MYSQL_PKGCONF_LIBRARY_DIRS}
	/usr/local/lib/mysql/
	/usr/lib/mysql/
	/sw/lib/mysql/
	/opt/local/lib/mysql55/mysql/
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(MYSQL_PROCESS_INCLUDES MYSQL_INCLUDE_DIR ZLIB_INCLUDE_DIRS)
set(MYSQL_PROCESS_LIBS MYSQL_LIBRARY ZLIB_LIBRARIES)
libfind_process(MYSQL)
