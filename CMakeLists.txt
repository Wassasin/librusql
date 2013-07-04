cmake_minimum_required(VERSION 2.8)

project(rusql)

include_directories("${PROJECT_BINARY_DIR}")

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
