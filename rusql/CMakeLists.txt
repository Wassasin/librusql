cmake_minimum_required(VERSION 2.8)

add_subdirectory(mysql)

file(GLOB sources *.cpp)
file(GLOB headers *.hpp)

source_group("rusql headers" FILES ${headers})

add_library(rusql ${sources} ${headers})
target_link_libraries(rusql rusql_mysql)

if(MYSQLd_FOUND)
  add_library(rusql_embedded ${sources})
  target_link_libraries(rusql_embedded rusql_mysql_embedded)
endif()

#TODO: Install rusql and rusql_embedded
