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
	test_init(2);
	db->execute("CREATE TABLE rusqltest (`value` INT(2) NOT NULL)");
	auto res = db->query("SHOW TABLES");
	test(contains(res, "rusqltest"), "rusqltest table created");
	db->execute("DROP TABLE rusqltest");
	res = db->query("SHOW TABLES");
	test(!contains(res, "rusqltest"), "rusqltest table dropped");
	return 0;
}
