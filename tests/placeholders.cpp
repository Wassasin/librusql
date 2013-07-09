#include <rusql/rusql.hpp>
#include "test.hpp"
#include "database_test.hpp"

int main(int argc, char *argv[]) {
	auto db = get_database(argc, argv);
	test_init(41);
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
	
	test_start_try(8);
	try {
		auto statement = db->execute("SELECT * FROM rusqltest");
		statement.store_result();

		std::string value;
		value.resize(10);
		statement.bind_results(value);
		test(statement.num_rows() == 3, "three results in num_rows()");
		test(statement.fetch(), "one result");
		test(value == "a", "a was inserted");
		test(statement.fetch(), "two results");
		test(value == "b", "b was inserted");
		test(statement.fetch(), "three results");
		test(value == "c", "c was inserted");
		test(!statement.fetch(), "end of results");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	test_start_try(2);
	try {
		// bound query without results
		auto statement = db->prepare("SELECT * FROM rusqltest WHERE value=?");
		statement.bind_parameters("foo'barbaz");
		statement.execute();
		test(statement.num_rows() == 0, "no results in num_rows()");
		statement.fetch();
		test(!statement, "no results in fetch()");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	test_start_try(7);
	try {
		auto res = db->query("SELECT * FROM rusqltest");
		test(res.num_rows() == 3, "3 results in resultset using num_rows()");
		test(res.get_string("value") == "a", "a was inserted");
		res.next();
		test(res, "two results");
		test(res.get_string("value") == "b", "b was inserted");
		res.next();
		test(res, "three results");
		test(res.get_string("value") == "c", "c was inserted");
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
	test_start_try(2);
	try {
		db->execute("INSERT INTO rusqltest VALUES (?)", 5);
		pass("Could insert numeric into string field");
		auto res = db->query("SELECT * FROM rusqltest");
		test(res.get_string(0) == "5", "Numeric was stored into string field correctly");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	db->execute("DELETE FROM rusqltest");
	db->execute("INSERT INTO rusqltest VALUES (?)", "6");
	test_start_try(2);
	try {
		auto res = db->query("SELECT * FROM rusqltest");
		auto r = res.get_uint64(0);
		pass("Could retrieve numeric from string field");
		test(r == 6, "Numeric was retrieved from string field correctly");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

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

	test_start_try(2);
	try {
		auto res = db->query("SELECT * FROM rusqltest");
		auto r = res.get_string(0);
		pass("Could retrieve string from numeric field");
		test(r == "3", "Retrieved string is correct");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	db->execute("DELETE FROM rusqltest");
	test_start_try(2);
	try {
		db->execute("INSERT INTO rusqltest VALUES (?)", "4");
		pass("Could insert string into numeric field");
		auto res = db->query("SELECT * FROM rusqltest");
		auto r = res.get_uint64(0);
		test(r == 4, "Inserted string into numeric field is correct");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

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
