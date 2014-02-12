#include <rusql/rusql.hpp>
#include "test.hpp"
#include "database_test.hpp"
#include <cstdlib>

static bool contains(rusql::ResultSet &rs, std::string value) {
	while(rs) {
		if(rs.get_string(0) == value) {
			return true;
		}
		rs.next();
	}
	return false;
}

int main(int argc, char *argv[]) {
	auto db = get_database(argc, argv);
	test_init(10);
	try {
		db->execute("CREATE TABLE rusqltest (`value` INT(2) NOT NULL)");
		pass("could run CREATE TABLE");
	} catch(std::exception &e) {
		diag(e);
		fail("could run CREATE TABLE");
	}
	test_start_try(1);
	try {
		auto res = db->query("SHOW TABLES");
		test(contains(res, "rusqltest"), "rusqltest table created");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	test_start_try(1);
	try {
		// SELECT on empty database
		auto res = db->query("SELECT * FROM rusqltest");
		test(!res, "no result rows in empty table");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	test_start_try(3);
	try {
		db->execute("INSERT INTO rusqltest (`value`) VALUES (27)");
		auto res = db->query("SELECT * FROM rusqltest");
		test(res, "result row in empty table");
		test(res.get_uint64(0) == 27, "correct result row in empty table");
		test(res.get_uint64("value") == 27, "correct result row by name in empty table");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	try {
		db->execute("DROP TABLE rusqltest");
		pass("could run DROP TABLE");
	} catch(std::exception &e) {
		diag(e);
		fail("could run DROP TABLE");
	}
	test_start_try(1);
	try {
		auto res = db->query("SHOW TABLES");
		test(!contains(res, "rusqltest"), "rusqltest table dropped");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	try {
		db->query("DROP TABLE nonexistant");
		fail("DROP nonexistant table throws");
	} catch(std::exception &e) {
		pass("DROP nonexistant table throws");
	}

	try {
		db->query("SELECT 1");
		pass("Next query succeeds too");
	} catch(std::exception &e) {
		fail("Next query succeeds too");
	}

	return 0;
}
