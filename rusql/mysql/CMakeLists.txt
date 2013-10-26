cmake_minimum_required(VERSION 2.8)

file(GLOB headers *.hpp)
file(GLOB sources *.cpp)

add_library(rusql_mysql ${sources} ${headers})
target_link_libraries(rusql_mysql ${MYSQL_LIBRARIES} ${Boost_LIBRARIES})

if(MYSQLd_FOUND)
  add_library(rusql_mysql_embedded ${sources})
  target_link_libraries(rusql_mysql_embedded ${MYSQLd_LIBRARIES} ${Boost_LIBRARIES})
endif()

install(FILES ${headers} DESTINATION include/rusql/mysql)

#TODO: Install rusql_mysql and rusql_mysql_embedded
