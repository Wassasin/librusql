#include <rusql/rusql.hpp>
#include "test.hpp"
#include "database_test.hpp"
#include <cstdint>

int main(int argc, char *argv[]) {
	auto db = get_database(argc, argv);
	test_init(4);

	const  int16_t negative_int = -16000;
	const uint16_t high_int     = 34000;

	db->execute("CREATE TABLE rusqltest (`value` SMALLINT(3) NOT NULL)");
	test_start_try(2);
	try {
		db->execute("INSERT INTO rusqltest VALUES (?)", negative_int);
		pass("store negative int");
		auto res = db->select_query("SELECT * FROM rusqltest");
		test(res.get<int16_t>(0) == negative_int, "retrieve negative int");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();
	db->execute("DROP TABLE rusqltest");

	db->execute("CREATE TABLE rusqltest (`value` SMALLINT(3) UNSIGNED NOT NULL)");
	test_start_try(2);
	try {
		db->execute("INSERT INTO rusqltest VALUES (?)", high_int);
		pass("store high int");
		auto res = db->select_query("SELECT * FROM rusqltest");
		diag(res.get_string(0));
		test(res.get<uint16_t>(0) == high_int, "retrieve high int");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();
	db->execute("DROP TABLE rusqltest");

	return 0;
}
