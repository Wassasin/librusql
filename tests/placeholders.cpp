#include <rusql/rusql.hpp>
#include "test.hpp"
#include "database_test.hpp"

int main(int argc, char *argv[]) {
	auto db = get_database(argc, argv);
	test_init(20);
	db->execute("CREATE TABLE rusqltest (`value` VARCHAR(10) NOT NULL)");

	test_start_try(6);
	try {
		db->execute("INSERT INTO rusqltest VALUES (?), (?), (?)", "a", "b", "c");
		auto res = db->query("SELECT * FROM rusqltest");
		test(res.get_string(0) == "a", "a was inserted");
		res.next();
		test(res, "two results");
		test(res.get_string(0) == "b", "b was inserted");
		res.next();
		test(res, "three results");
		test(res.get_string(0) == "c", "c was inserted");
		res.next();
		test(!res, "not more than three results");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	db->execute("DELETE FROM rusqltest");
	try {
		db->execute("INSERT INTO rusqltest VALUES (?)", "a", "b");
		fail("Too many placeholders fails");
	} catch(std::exception &) {
		pass("Too many placeholders fails");
	}

	try {
		db->execute("INSERT INTO rusqltest VALUES (?), (?)", "a");
		fail("Too few placeholders fails");
	} catch(std::exception &) {
		pass("Too few placeholders fails");
	}

	db->execute("DELETE FROM rusqltest");
	try {
		db->execute("INSERT INTO rusqltest VALUES (?)", 5);
		auto res = db->query("SELECT * FROM rusqltest");
		diag("Actually stored string: '" + res.get_string(0) + "'");
		fail("Disallowed to enter numerics in string field");
	} catch(std::exception &) {
		pass("Disallowed to enter numerics in string field");
	}

	db->execute("DELETE FROM rusqltest");
	db->execute("INSERT INTO rusqltest VALUES (?)", "6");
	try {
		auto res = db->query("SELECT * FROM rusqltest");
		auto r = res.get_uint64(0);
		diag("Actually retrieved numeric: " + to_string(r));
		fail("Disallowed to retrieve numerics from string field");
	} catch(std::exception &) {
		pass("Disallowed to retrieve numerics from string field");
	}

	db->execute("DROP TABLE rusqltest");
	db->execute("CREATE TABLE rusqltest (`value` INT(2) NOT NULL)");
	test_start_try(2);
	try {
		db->execute("INSERT INTO rusqltest VALUES (?)", 3);
		auto res = db->query("SELECT * FROM rusqltest");
		test(res.get_uint64(0) == 3, "3 was inserted");
		res.next();
		test(!res, "not more than one result");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	try {
		auto res = db->query("SELECT * FROM rusqltest");
		auto r = res.get_string(0);
		diag("Actually retrieved string: '" + r + "'");
		fail("Disallowed to retrieve strings from numeric field");
	} catch(std::exception&) {
		pass("Disallowed to retrieve strings from numeric fields");
	}

	db->execute("DELETE FROM rusqltest");
	try {
		db->execute("INSERT INTO rusqltest VALUES (?)", "4");
		auto res = db->query("SELECT * FROM rusqltest");
		auto r = res.get_uint64(0);
		diag("Actually retrieved numeric: " + to_string(r));
		fail("Disallowed to enter strings in numeric field");
	} catch(std::exception&) {
		pass("Disallowed to enter strings in numeric field");
	}

	db->execute("DROP TABLE rusqltest");
	db->execute("CREATE TABLE rusqltest(`value` INT(2) NULL)");
	test_start_try(3);
	try {
		db->execute("INSERT INTO rusqltest VALUES (?)", 2);
		pass("Insert values into NULL field");
		auto res = db->query("SELECT * FROM rusqltest");
		bool r = res.is_null(0);
		pass("Could ask if value is null");
		test(!r, "Value inserted from 2 was not null");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();
	db->execute("DELETE FROM rusqltest");
	test_start_try(3);
	try {
		db->execute("INSERT INTO rusqltest VALUES (?)", boost::none);
		pass("Insert boost::none");
		auto res = db->query("SELECT * FROM rusqltest");
		bool r = res.is_null(0);
		pass("Could ask if value is null");
		test(r, "Value inserted from none was null");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	db->execute("DROP TABLE rusqltest");

	return 0;
}
