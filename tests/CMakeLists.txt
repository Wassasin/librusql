set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/../cmake/modules")
find_package(MysqlCppConn)
include_directories(${MysqlCppConn_INCLUDE_DIRS})
#  MysqlCppConn_LIBRARIES	- libraries for MysqlCppConn
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
add_definitions("--std=c++0x -O0 -Wall -Wno-long-long -DNDEBUG")

add_executable(compile ${CMAKE_CURRENT_SOURCE_DIR}/compile.cpp)
target_link_libraries(compile ${MysqlCppConn_LIBRARIES})
add_executable(connect ${CMAKE_CURRENT_SOURCE_DIR}/connect.cpp)
target_link_libraries(connect ${MysqlCppConn_LIBRARIES})
