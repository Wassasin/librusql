include(${CMAKE_CURRENT_LIST_DIR}/detect_compiler.cmake)
function(get_sane_warning_flags result_var)
	if(NOT DEFINED ${result_var})
		check_compiling_with_clang(COMPILING_WITH_CLANG)
		check_compiling_with_gcc(COMPILING_WITH_GCC)
		if(${COMPILING_WITH_CLANG})
			#We enable all the warnings, and then whitelist some
			list(APPEND warnings -Weverything)
			#Low level stuff
			list(APPEND warnings -Wno-padded -Wno-weak-vtables)
			#Alignment warnings
			list(APPEND warnings  -Wno-cast-align -Wno-over-aligned)
			#Pedantic stuff
			list(APPEND warnings -Wno-newline-eof -Wno-pedantic -Wno-missing-prototypes -Wno-missing-noreturn -Wno-extra-semi  -Wno-covered-switch-default -Wno-unreachable-code)
			#Pre-C++11 compatibility can be screwed
			list(APPEND warnings -Wno-c++98-compat-pedantic)
			#C99 stuff that most compilers did add to C++
			list(APPEND warnings -Wno-variadic-macros -Wno-disabled-macro-expansion -Wno-vla)
			#Globals, perhaps rethink this since they can really cause issues if they depend on eachother.
			list(APPEND warnings -Wno-global-constructors -Wno-exit-time-destructors)
			#Conversion from double -> float, most of the time perfectly safe
			list(APPEND warnings -Wno-conversion)
			#Float equalness can't be checked with ==, but we know that right?
			list(APPEND warnings -Wno-float-equal)
		elseif(${COMPILING_WITH_GCC})
			#default stuff
			list(APPEND warnings -Wall) #-Weffc++ and Wextra are disabled because of internal compiler errors on gcc 4.7.2
			#switch related warnings
			list(APPEND warnings -Wswitch-default -Wswitch-enum)
			#missing struct member initialisation warnings
			list(APPEND warnings -Wno-missing-braces -Wno-missing-field-initializers)
			#low level warnings
			list(APPEND warnings -Wstrict-overflow=5 -Wcast-align -Wstrict-aliasing=2)
			#suspicious coding detection
			list(APPEND warnings -Wshadow -Wredundant-decls -Wunreachable-code -Wlogical-op -Wundef -Wformat=2 -Wpointer-arith)
		endif()
		set(${result_var} ${warnings} PARENT_SCOPE)
	endif()
endfunction(get_sane_warning_flags)
