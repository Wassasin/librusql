include(${CMAKE_CURRENT_LIST_DIR}/detect_compiler.cmake)
check_compiling_with_clang(COMPILING_WITH_CLANG)
check_compiling_with_gcc(COMPILING_WITH_GCC)

# If the chosen C++ compiler is unable to link with the -std=c++0x flag, give
# a fatal error
INCLUDE(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_FLAGS "-std=c++0x")

if(APPLE)
	set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -stdlib=libc++")
endif(APPLE)

CHECK_CXX_SOURCE_COMPILES(
	"int main(int argc, char *argv[]) {
		#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
			return 1;
		#else
			\"Compilation should fail, C++0x is not supported\"
		#endif
	}"

	CPP0X_SUPPORTED
)

if(NOT CPP0X_SUPPORTED)
	message(FATAL_ERROR "Your chosen compiler does not support C++0x/C++11, this project needs it.")
endif()
