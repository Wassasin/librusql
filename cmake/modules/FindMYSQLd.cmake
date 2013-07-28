# - Find MySQL-embedded-server
# This module sets
#
# MYSQLd_FOUND
# MYSQLd_INCLUDE_DIR
# MYSQLd_LIBRARIES

include(LibFindMacros)

libfind_package(MYSQLd zlib)

libfind_pkg_check_modules(MYSQLd_PKGCONF mysqld)

find_path(MYSQLd_INCLUDE_DIR
	NAMES mysql.h
	PATHS
	${MYSQLd_PKGCONF_INCLUDE_DIRS}
	/usr/local/include/mysql/
	/usr/include/mysql/
	/sw/include/mysql/
	/opt/local/include/mysql55/mysql/
)

find_library(MYSQLd_LIBRARY
	NAMES mysqld
	PATHS ${MYSQLd_PKGCONF_LIBRARY_DIRS}
	/usr/local/lib/mysql/
	/usr/lib/mysql/
	/sw/lib/mysql/
	/opt/local/lib/mysql55/mysql/
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(MYSQLd_PROCESS_INCLUDES MYSQLd_INCLUDE_DIR ZLIB_INCLUDE_DIRS)
set(MYSQLd_PROCESS_LIBS MYSQLd_LIBRARY ZLIB_LIBRARIES)
libfind_process(MYSQLd)
