#include <rusql/rusql.hpp>
#include "test.hpp"
#include "database_test.hpp"
#include <cstdlib>

static void add_product(std::shared_ptr<rusql::Database> db, int one, int two) {
	db->execute("INSERT INTO rusqltest VALUES (?, ?, ?)", one, two, one * two);
}

int main(int argc, char *argv[]) {
	auto db = get_database(argc, argv);
	db->execute("CREATE TABLE rusqltest (`one` INT(10) NOT NULL, `two` INT(10) NOT NULL, `product` INT(10) NOT NULL)");
	add_product(db, 5, 7);
	add_product(db, 11, 13);
	add_product(db, 17, 19);
	add_product(db, 1009, 509);
	add_product(db, 2609, 2);
	const int num_products = 5;
	test_init(9 + num_products * 4);

	// set up three connections, iterate through them simultaneously
	auto rsOne  = db->query("SELECT one FROM rusqltest");
	test(db->number_of_active_connections() >= 1, "one connection active");
	int active_connections_before;
	{
		auto rsTwo  = db->query("SELECT two FROM rusqltest");
		test(db->number_of_active_connections() >= 2, "two connections active");
		auto rsProd = db->query("SELECT product FROM rusqltest");
		test(db->number_of_active_connections() >= 3, "three connections active");

		for(int i = 0; i < num_products; ++i) {
			test(rsOne,  "connection one still has results");
			test(rsTwo,  "connection two still has results");
			test(rsProd, "connection three still has results");
			int one  = rsOne.get_uint64("one");
			int two  = rsTwo.get_uint64("two");
			int prod = rsProd.get_uint64("product");
			test(one * two == prod, "connections are simultaneous");
			rsOne.next();
			rsTwo.next();
			rsProd.next();
		}
		test(!rsOne,  "connection one at end of results");
		test(!rsTwo,  "connection two at end of results");
		test(!rsProd, "connection three at end of results");
		active_connections_before = db->number_of_active_connections();
		test(db->number_of_active_connections() >= 3, "three connections still active");
	}
	test(active_connections_before > db->number_of_active_connections(), "freeing result sets decreases number of active connections");
	test(db->number_of_active_connections() >= 1, "one connection still active");

	db->execute("DROP TABLE rusqltest");
	return 0;
}
