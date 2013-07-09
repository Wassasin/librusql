#include <rusql/rusql.hpp>
#include "test.hpp"
#include "database_test.hpp"
#include <cstdlib>

int main(int argc, char *argv[]) {
	auto db = get_database(argc, argv);
	test_init(8);
	test_start_try(4);
	try {
		db->execute("CREATE TABLE rusqltest (`id` INTEGER(10) PRIMARY KEY AUTO_INCREMENT, `value` INT(2) NOT NULL)");
		db->execute("INSERT INTO rusqltest (`value`) VALUES (1)");
		auto &connection = db->get_connection();
		connection.execute("INSERT INTO rusqltest (`value`) VALUES (33)");
		uint64_t insert_id = connection.insert_id();
		auto statement = db->prepare("SELECT id FROM rusqltest WHERE value=33");
		statement.execute();
		uint64_t real_insert_id = 0xdeadbeef;
		statement.bind_results(real_insert_id);
		test(statement.fetch(), "one result");
		test(real_insert_id != 0xdeadbeef, "real_insert_id was changed");
		test(real_insert_id == insert_id, "last_insert_id() returned correctly");
		test(!statement.fetch(), "one result only");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	test_start_try(4);
	try {
		auto statement = db->prepare("INSERT INTO rusqltest (`value`) VALUES (67)");
		statement.execute();
		uint64_t insert_id = statement.insert_id();
		auto st2 = db->prepare("SELECT id FROM rusqltest WHERE value=67");
		st2.execute();
		uint64_t real_insert_id = 0xdeadbeef;
		st2.bind_results(real_insert_id);
		test(st2.fetch(), "one result");
		test(real_insert_id != 0xdeadbeef, "real_insert_id was changed");
		test(real_insert_id == insert_id, "last_insert_id() returned correctly");
		test(!st2.fetch(), "one result only");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	db->execute("DROP TABLE rusqltest");
	return 0;
}
