#include <rusql/rusql.hpp>
#include <vector>
#include "test.hpp"
#include "database_test.hpp"

int main(int argc, char *argv[]) {
	auto db = get_database(argc, argv);
	test_init(13);

	test_start_try(3);
	try {
		db->execute("CREATE TABLE rusqltest (`id` INT(10) NOT NULL, `value` VARCHAR(10) NOT NULL)");
		auto statement = db->prepare("INSERT INTO rusqltest VALUES (?, ?)");
		statement.execute(1, "a");
		pass("inserted first row");
		statement.execute(2, "bc");
		pass("inserted second row");
		statement.execute(3, "def");
		pass("inserted third row");
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	struct Row {
		int id;
		std::string value;
	};

	std::vector<Row> rows;
	test_start_try(4);
	try {
		auto statement = db->execute("SELECT * FROM rusqltest");
		int i = 0;
		while(++i <= 4) {
			rows.resize(rows.size() + 1);
			Row &this_row = rows.back();
			statement.bind_results(this_row.id, this_row.value);
			bool fetched = statement.fetch();
			if(i <= 3) {
				test(fetched, "fetch worked");
			} else {
				test(!fetched, "fetch stopped at the right time");
			}
		}
	} catch(std::exception &e) {
		diag(e);
	}
	test_finish_try();

	rows.resize(3);
	test(rows[0].id == 1, "id 1 correct");
	test(rows[0].value == "a", "value 1 correct");
	test(rows[1].id == 2, "id 2 correct");
	test(rows[1].value == "bc", "value 2 correct");
	test(rows[2].id == 3, "id 3 correct");
	test(rows[2].value == "def", "value 3 correct");

	return 0;
}
