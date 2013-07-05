#include <rusql/rusql.hpp>
#include <iostream>
#include "test.hpp"
#include "database_test.hpp"

int main(int argc, char *argv[]) {
	std::shared_ptr<rusql::Database> db;
	try {
		// on invalid parameters, this method skips all tests and calls exit()
		// on failed connection, this method should throw
		db = get_database(argc, argv);
	} catch(std::exception &e) {
		diag(e);
	}

	test_init(2);
	test(db.get() != 0, "Connected");

	try {
		db->ping();
		pass("Ping");
	} catch(std::exception &e) {
		diag(e);
		fail("Ping threw");
	}
}
