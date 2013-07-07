cmake_minimum_required(VERSION 2.8)

project(rusql)

include(cmake/set_system_include_flag.cmake)
include(cmake/address_sanitizer.cmake)

include(cmake/detect_compiler.cmake)
check_compiling_with_clang(COMPILING_WITH_CLANG)
check_compiling_with_gcc(COMPILING_WITH_GCC)

if(${COMPILING_WITH_CLANG})
	if(APPLE)
		include_directories(SYSTEM "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/c++/v1/")
	endif()
endif()

if(MINGW)
	include(cmake/enable_gnu11.cmake)
else()
	include(cmake/enable_cpp11.cmake)
endif()

if(UNIX)
	#Always compile with debugging symbols
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")
else()
	if(MSVC)
		message(FATAL_ERROR "Were not supporting MSVC at this time")
	elseif(MINGW)
		# Assume were running cross compilation.
		add_definitions("-DGLEW_STATIC" "-DAL_LIBTYPE_STATIC" "-DALURE_STATIC_LIBRARY" "-DBOOST_THREAD_USE_LIB")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")
	else()
		message(FATAL_ERROR "Unsupported platform detected")
	endif()
endif()

include(cmake/warning_settings.cmake)
get_sane_warning_flags(warnings)

include(cmake/join.cmake)
join("${warnings}" " " warnings)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

add_subdirectory(rusql)
add_subdirectory(tests)
