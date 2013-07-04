
#TODO: Fix this, use Dakon's detect C++11 module git://anongit.kde.org/scratch/dakon/cmake-cxx11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
if(APPLE)
	message(FATAL_ERROR "We cannot enable gnu extensions on mac OS X")
endif(APPLE)
