#include <rusql/rusql.hpp>
#include <boost/optional.hpp>
#include "test.hpp"
#include "database_test.hpp"

int main(int argc, char *argv[]) {
	auto db = get_database(argc, argv);
	test_init(22);

	// TODO: after named_bind() is added, throw if it is called without execute()
	// TODO: throw if get() was called without fetch()
	// TODO: throw if get() was called without bind_results() (or once it's added, named_bind())
	// TODO: throw if get() was called with a nonexistant field

	db->execute("CREATE TABLE rusqltest (`id` INT(10) NOT NULL, `value` VARCHAR(10) NOT NULL)");
	db->execute("INSERT INTO rusqltest VALUES (?, ?), (?, ?), (?, ?)", 5, "a", 6, "b", 7, "c");

	test_start_try(10);
	try {
		auto statement = db->execute("SELECT * FROM rusqltest");

		statement.bind_all_self();

		test(statement.fetch(), "first result");
		test(statement.get<int>("id") == 5, "first result int");
		test(statement.get<std::string>("value") == "a", "first result string");
		test(statement.fetch(), "second result");
		test(statement.get<int>("id") == 6, "second result int");
		test(statement.get<std::string>("value") == "b", "second result string");
		test(statement.fetch(), "third result");
		test(statement.get<std::string>("value") == "c", "third result string");
		test(statement.get<int>("id") == 7, "third result int");
		test(!statement.fetch(), "end of results");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	// test NULL columns
	db->execute("DROP TABLE rusqltest");
	db->execute("CREATE TABLE rusqltest (`id` INT(10) NOT NULL, `value` VARCHAR(10) NULL)");
	db->execute("INSERT INTO rusqltest VALUES (?, ?), (?, NULL), (?, NULL)", 5, "a", 6, 7);

	test_start_try(12);
	try {
		auto statement = db->execute("SELECT * FROM rusqltest");

		statement.bind_all_self();

		test(statement.fetch(), "first result");
		test(statement.get<int>("id") == 5, "first result int");
		test(statement.get<std::string>("value") == "a", "first result string");
		boost::optional<std::string> placeholder;
		placeholder = statement.get<decltype(placeholder)>("value");
		test(placeholder, "first result string is set");
		test(*placeholder == "a", "first result string is correct");

		test(statement.fetch(), "second result");
		test(statement.get<int>("id") == 6, "second result int");
		placeholder = statement.get<decltype(placeholder)>("value");
		test(!placeholder, "second result string is not set");

		// TODO: this should throw someday
		/*bool threw = false;
		try {
			statement.get<std::string>("value");
		} catch(std::exception &e) {
			threw = true;
			diag(e);
		}
		test(threw, "get<string> doesn't return");*/

		test(statement.fetch(), "third result");
		placeholder = statement.get<decltype(placeholder)>("value");
		test(!placeholder, "third result string is not set");
		test(statement.get<int>("id") == 7, "third result int");
		test(!statement.fetch(), "end of results");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	return 0;
}
