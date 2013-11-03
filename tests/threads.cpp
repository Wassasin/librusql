#include <rusql/rusql.hpp>
#include <boost/thread.hpp>
#include "test.hpp"
#include "database_test.hpp"

int main(int argc, char *argv[]) {
	auto db = get_database(argc, argv);
	test_init(6);
	db->execute("CREATE TABLE rusqltest (`value` INT(2) NOT NULL)");
	db->execute("INSERT INTO rusqltest VALUES (20)");

	auto statement = db->prepare("SELECT value FROM rusqltest");

	statement.execute();

	boost::thread thread([&db]() {
		auto thread_handle = db->get_thread_handle();
		db->execute("UPDATE rusqltest SET value=30");
	});
	thread.join();

	uint64_t value = 10;
	statement.bind_results(value);
	test(statement.fetch(), "one result");
	test(value == 20, "value was correct (" + std::to_string(value) + ")");
	test(!statement.fetch(), "exactly one result");

	statement.execute();
	statement.bind_results(value);
	test(statement.fetch(), "one result");
	test(value == 30, "value was correct (" + std::to_string(value) + ")");
	test(!statement.fetch(), "exactly one result");

	db->execute("DROP TABLE rusqltest");
}
