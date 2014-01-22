include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")

if(MYSQLd_FOUND)

add_custom_target(check COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tests.pl"
COMMENT "\nTo run the tests against a live database, call:\n${CMAKE_CURRENT_SOURCE_DIR}/tests.pl <host> <user> <pass> <emptydb>")

foreach(TEST compile connect optional placeholders query multiconnection signedness insert_id iterate threads named_bind)
	add_executable(test_${TEST} EXCLUDE_FROM_ALL ${CMAKE_CURRENT_SOURCE_DIR}/${TEST}.cpp)
	target_link_libraries(test_${TEST} rusql_embedded)
	add_test(test_${TEST} test_${TEST})
	add_dependencies(check test_${TEST})
endforeach()

endif(MYSQLd_FOUND)
