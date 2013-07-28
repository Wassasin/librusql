cmake_minimum_required(VERSION 2.8)

add_subdirectory(mysql)
include_directories(${MYSQL_DRIVER_INCLUDE_DIRS})

file(GLOB sources *.cpp)
file(GLOB headers *.hpp)

source_group("rusql headers" FILES ${headers})

add_library(rusql ${sources} ${headers})
target_link_libraries(rusql rusql_mysql)

find_package(MYSQLd)
if(MYSQLd_FOUND)
  add_library(rusql_embedded ${sources})
  target_link_libraries(rusql_embedded rusql_mysql_embedded)
endif()

set(RUSQL_INCLUDE_DIRS ${MYSQL_DRIVER_INCLUDE_DIRS} PARENT_SCOPE)

#TODO: Install rusql and rusql_embedded
