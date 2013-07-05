include(${CMAKE_CURRENT_LIST_DIR}/detect_compiler.cmake)
check_compiling_with_clang(COMPILING_WITH_CLANG)
check_compiling_with_gcc(COMPILING_WITH_GCC)

if(COMPILING_WITH_CLANG OR COMPILING_WITH_GCC)
	set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem")
endif()
