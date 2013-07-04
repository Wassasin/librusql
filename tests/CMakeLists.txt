add_definitions("-std=c++0x -O0 -Wall -Wno-long-long -DNDEBUG")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../src")

enable_testing()

foreach(TEST compile connect optional placeholders query)
	add_executable(test_${TEST} ${CMAKE_CURRENT_SOURCE_DIR}/${TEST}.cpp)
	target_link_libraries(test_${TEST} rusql)
	add_test(test_${TEST} test_${TEST})
endforeach()
