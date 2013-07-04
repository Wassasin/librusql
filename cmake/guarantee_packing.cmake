
#for now this is only an issue on mingw but should it become an issue on another platform please append it to this file
if(MINGW)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-ms-bitfields")
endif()
