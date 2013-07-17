#include <rusql/rusql.hpp>
#include <boost/optional.hpp>
#include "test.hpp"
#include "database_test.hpp"

template <typename T>
static bool contains(rusql::ResultSet &rs, const T &value) {
	while(rs) {
		if(rs.get<T>(size_t(0)) == value) {
			return true;
		}
		rs.next();
	}
	return false;
}

int main(int argc, char *argv[]) {
	auto db = get_database(argc, argv);
	test_init(18);
	db->execute("CREATE TABLE rusqltest (`value` VARCHAR(10) NULL)");

	test_start_try(1);
	try {
		db->execute("INSERT INTO rusqltest VALUES (?)", boost::optional<std::string>("foo"));
		auto res = db->query("SELECT * FROM rusqltest");
		test(contains(res, std::string("foo")), "could insert string from optional");
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

		boost::optional<std::string> value("some initialized value");
		statement.bind_results(value);
		test(statement.num_rows() == 2, "two results in num_rows()");
		test(statement.fetch(), "first result");
		test(!value, "first result is null");
		test(statement.fetch(), "second results");
		test(value, "second result is set");
		test(*value == "a", "second result is set correctly");
		test(!statement.fetch(), "end of results");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	db->execute("DROP TABLE rusqltest");
	db->execute("CREATE TABLE rusqltest (value INT(10) NULL)");

	test_start_try(7);
	try {
		db->execute("INSERT INTO rusqltest VALUES (?)", boost::optional<int>());
		db->execute("INSERT INTO rusqltest VALUES (?)", boost::optional<int>(5));
		auto statement = db->execute("SELECT * FROM rusqltest");
		statement.store_result();

		boost::optional<int> value = 0xdeadbeef;
		statement.bind_results(value);
		test(statement.num_rows() == 2, "two results in num_rows after inserting optional ints");
		test(statement.fetch(), "first result");
		test(!value, "first result is null");
		test(statement.fetch(), "second result");
		test(value, "second result is set");
		test(*value == 5, "second result is set correctly");
		test(!statement.fetch(), "end of results");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	return 0;
}
