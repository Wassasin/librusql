#include <rusql/rusql.hpp>
#include <iostream>
#include "test.hpp"
#include "database_test.hpp"

int main(int argc, char *argv[]) {
	test_init(2);
	test_start_try(2);
	try {
		auto db = get_database(argc, argv);
		pass("Constructor finished");
		fail("Ping not implemented");
		//test(db->ping(), "Ping");
	} catch(std::exception &e) {
	}
	test_finish_try();
}
