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
	test_init(4);
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

	db->execute("DROP TABLE rusqltest");
	return 0;
}
