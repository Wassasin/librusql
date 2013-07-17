#include <rusql/rusql.hpp>
#include <boost/optional.hpp>
#include "test.hpp"
#include "database_test.hpp"

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
	test_init(11);
	db->execute("CREATE TABLE rusqltest (`value` VARCHAR(10) NULL)");

	test_start_try(1);
	try {
		db->execute("INSERT INTO rusqltest VALUES (?)", boost::optional<std::string>("foo"));
		auto res = db->query("SELECT * FROM rusqltest");
		test(contains(res, "foo"), "could insert string from optional");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	db->execute("DELETE FROM rusqltest");
	test_start_try(2);
	try {
		db->execute("INSERT INTO rusqltest VALUES (?)", boost::optional<std::string>());
		auto res = db->query("SELECT * FROM rusqltest");
		bool null = res.is_null(0);
		pass("could ask if value was null");
		test(null, "value inserted from empty optional was null");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	try {
		auto res = db->query("SELECT * FROM rusqltest");
		auto r = res.get_string(0);
		diag("Result of get_string on null: '" + r + "'");
		fail("get_string on null throws");
	} catch(std::exception&) {
		pass("get_string on null throws");
	}

	test_start_try(7);
	try {
		db->execute("INSERT INTO rusqltest VALUES (?)", "a");

		auto statement = db->execute("SELECT * FROM rusqltest");
		statement.store_result();

		// TODO: I don't want to have to pre-allocate enough space.
		boost::optional<std::string> value = std::string("enough space for the next value");
		statement.bind_results(value);
		test(statement.num_rows() == 2, "two results in num_rows()");
		test(statement.fetch(), "first result");
		test(!value, "first result is null");
		value = std::string("enough space for the next value");
		test(statement.fetch(), "second results");
		test(value, "second result is set");
		test(*value == "a", "second result is set correctly");
		test(!statement.fetch(), "end of results");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	db->execute("DROP TABLE rusqltest");
	return 0;
}
