cmake_minimum_required(VERSION 2.8)

file(GLOB headers *.hpp)
file(GLOB sources *.cpp)

find_package(MYSQL REQUIRED)
include_directories(${MYSQL_INCLUDE_DIRS})
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_library(rusql_mysql ${sources})
target_link_libraries(rusql_mysql ${MYSQL_LIBRARIES} ${Boost_LIBRARIES})

if(MYSQLd_FOUND)
  add_library(rusql_mysql_embedded ${sources})
  target_link_libraries(rusql_mysql_embedded ${MYSQLd_LIBRARIES} ${Boost_LIBRARIES})
endif()

install(FILES ${headers} DESTINATION include/rusql/mysql)
#TODO: Install rusql_mysql and rusql_mysql_embedded
install(FILES
	cmake/modules/FindMYSQL.cmake
	cmake/modules/Findrusql.cmake
	DESTINATION share/cmake-2.8/Modules
)
