
#TODO: Fix this, use Dakon's detect C++11 module git://anongit.kde.org/scratch/dakon/cmake-cxx11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
if(APPLE)
	set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -stdlib=libc++")
endif(APPLE)
